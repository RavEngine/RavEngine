#if RGL_VK_AVAILABLE
#include "VkSwapchain.hpp"
#include "RGLVk.hpp"
#include "VkSynchronization.hpp"
#include <algorithm>
#undef min
#undef max

RGL::SwapchainVK::~SwapchainVK(){
    DestroySwapchainIfNeeded();
    vkDestroySemaphore(owningDevice->device, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(owningDevice->device, renderCompleteSemaphore, nullptr);
}

RGL::SwapchainVK::SwapchainVK(decltype(owningSurface) surface, decltype(owningDevice) owningDevice, int width, int height) : owningSurface(surface), owningDevice(owningDevice)
{
    VkSemaphoreCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0
    };
    VK_CHECK(vkCreateSemaphore(owningDevice->device, &info, nullptr, &imageAvailableSemaphore));
    VK_CHECK(vkCreateSemaphore(owningDevice->device, &info, nullptr, &renderCompleteSemaphore));
    Resize(width, height);
}

void RGL::SwapchainVK::Resize(uint32_t width, uint32_t height)
{
    // kill the old one
    DestroySwapchainIfNeeded();

    constexpr auto chooseSwapSurfaceFormat = [](const std::vector<VkSurfaceFormatKHR>& availableFormats) -> VkSurfaceFormatKHR {
        // we want BGRA8 Unorm in nonlinear space
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        //otherwise hope the first one is good enough
        return availableFormats[0];
    };
    constexpr auto chooseSwapPresentMode = [](const std::vector<VkPresentModeKHR>& availablePresentModes) -> VkPresentModeKHR {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;    // use Mailbox on high-perf devices
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;        // otherwise use FIFO when on low-power devices, like a mobile phone
    };
    auto chooseSwapExtent = [width,height](const VkSurfaceCapabilitiesKHR& capabilities) ->VkExtent2D {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    };

    // configure the swap chain stuff
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(owningDevice->physicalDevice,owningSurface->surface);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;  // we want one extra image than necessary to reduce latency (no waiting for the driver)
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR swapchainCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = owningSurface->surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,      // always 1 unless we are doing stereoscopic 3D
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,   // use VK_IMAGE_USAGE_TRANSFER_DST_BIT for offscreen rendering
        .preTransform = swapChainSupport.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,     // we don't care about pixels that are obscured
        .oldSwapchain = VK_NULL_HANDLE  // future issue
    };
    auto& indices = owningDevice->indices;
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    if (indices.graphicsFamily != indices.presentFamily) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0; // Optional
        swapchainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    VK_CHECK(vkCreateSwapchainKHR(owningDevice->device, &swapchainCreateInfo, nullptr, &swapChain));

    // remember these values
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    // get the swap chain images
    vkGetSwapchainImagesKHR(owningDevice->device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(owningDevice->device, swapChain, &imageCount, swapChainImages.data());


    // create image views from images
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapChainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapChainImageFormat,
            .components{
                .r = VK_COMPONENT_SWIZZLE_IDENTITY, // we don't want any swizzling
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,    // mipmap and layer info (we don't want any here)
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        auto debugName = std::string("swapchain ") + std::to_string(i);

        owningDevice->SetDebugNameForResource(createInfo.image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, debugName.c_str());


        VK_CHECK(vkCreateImageView(owningDevice->device, &createInfo, nullptr, &swapChainImageViews[i]));

        auto viewDebugName = std::string("swapchain image view ") + std::to_string(i);
        owningDevice->SetDebugNameForResource(swapChainImageViews[i], VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, viewDebugName.c_str());

        auto& texture = RGLTextureResources.emplace_back(swapChainImageViews[i], swapChainImages[i], Dimension{ width,height });
        texture.owningSwapchain = this;
    }
}

void RGL::SwapchainVK::GetNextImage(uint32_t* index)
{
    vkAcquireNextImageKHR(owningDevice->device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, index);
}

void RGL::SwapchainVK::Present(const SwapchainPresentConfig& config)
{
    VkSwapchainKHR swapChains[] = { swapChain };
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderCompleteSemaphore,
        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = &(config.imageIndex),
        .pResults = nullptr         // optional
    };
    auto result = vkQueuePresentKHR(owningDevice->presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        auto size = RGLTextureResources[0].GetSize();
        Resize(size.width, size.height);
    }
    else if (result != VK_SUCCESS) {
        FatalError("Failed to present swapchain image");
    }
}


void RGL::SwapchainVK::DestroySwapchainIfNeeded()
{
    if (swapChain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(owningDevice->device, swapChain, nullptr);
        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(owningDevice->device, imageView, nullptr);
        }
    }
    RGLTextureResources.clear();
}

#endif
