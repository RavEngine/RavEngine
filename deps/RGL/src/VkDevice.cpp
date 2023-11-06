#if RGL_VK_AVAILABLE
#include "VkDevice.hpp"
#include "RGLVk.hpp"
#include "VkSwapchain.hpp"
#include "VkRenderPass.hpp"
#include "VkRenderPipeline.hpp"
#include "VkShaderLibrary.hpp"
#include "VkBuffer.hpp"
#include "VkCommandQueue.hpp"
#include "VkSynchronization.hpp"
#include "VkTexture.hpp"
#include "VkSampler.hpp"
#include "VkComputePipeline.hpp"
#include <vector>
#include <stdexcept>
#include <set>
#include <vulkan/vulkan.h>

namespace RGL {

    template<typename T>
    void loadVulkanFunction(VkDevice device, T& ptr, const char* fnname) {
        ptr = (std::remove_reference_t<decltype(ptr)>) vkGetDeviceProcAddr(device, fnname);
        if (!ptr) {
            FatalError(std::string("Cannot get Vulkan function pointer: ") + fnname);
        }
    }

    constexpr static const char* const deviceExtensions[] = {
           VK_KHR_SWAPCHAIN_EXTENSION_NAME,
           VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME,
           VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME,
           VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
           VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
           VK_KHR_MAINTENANCE1_EXTENSION_NAME,
           VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
           VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
           VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
           VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
           VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME,
           VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME,
           VK_EXT_MEMORY_BUDGET_EXTENSION_NAME
    };

    bool checkDeviceExtensionSupport(const VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(std::begin(deviceExtensions), std::end(deviceExtensions));

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    };

    // find a queue of the right family
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;
        // Logic to find queue family indices to populate struct with

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                indices.graphicsFamily = i;
                indices.presentFamily = i;  // note: to do this properly, one should use vkGetPhysicalDeviceSurfaceSupportKHR and check surface support, but we don't have a surface
                // in general graphics queues are able to present. if the use has different needs, they should not use the default device.
            }

            /*VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) {
                indices.presentFamily = i;
            }*/
            i++;
        }
        return indices;
    };


    RGLDevicePtr CreateDefaultDeviceVk() {
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        // now select and configure a device
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        Assert(deviceCount != 0, "No GPUs with Vulkan support");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());


        constexpr auto isDeviceSuitable = [](const VkPhysicalDevice device) -> bool {
            // look for all the features we want
            VkPhysicalDevicePushDescriptorPropertiesKHR devicePushDescriptors{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR,
                .pNext = nullptr
            };

            VkPhysicalDeviceProperties2 deviceProperties{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
                .pNext = &devicePushDescriptors,
            };
            vkGetPhysicalDeviceProperties2(device, &deviceProperties);

            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            auto queueFamilyData = findQueueFamilies(device);

            auto extensionsSupported = checkDeviceExtensionSupport(device);

            bool swapChainAdequate = false;
            if (extensionsSupported) {
                /*SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
                bool swapchainSupported = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();*/
                swapChainAdequate = /*swapchainSupported &&*/ swapChainAdequate;
            }

            // right now we don't care so pick any gpu
            // in the future implement a scoring system to pick the best device
            return deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && queueFamilyData.isComplete() && extensionsSupported;
        };
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }
        Assert(physicalDevice != VK_NULL_HANDLE, "failed to find a suitable GPU!");

        return std::make_shared<DeviceVk>(physicalDevice);
    }

    RGL::DeviceVk::DeviceVk(decltype(physicalDevice) physicalDevice) : physicalDevice(physicalDevice) {
        // next create the logical device and the queue
        indices = findQueueFamilies(physicalDevice);
        float queuePriority = 1.0f;     // required even if we only have one queue. Used to cooperatively schedule multiple queues

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
            };
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{
            .samplerAnisotropy = VK_TRUE,   // need to explicity request it
        };

        VkPhysicalDeviceVulkan13Features vulkan1_3Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = nullptr
        };
        
        VkPhysicalDeviceVulkan12Features vulkan1_2Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &vulkan1_3Features
        };
        VkPhysicalDeviceCustomBorderColorFeaturesEXT customBorderColor{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT,
            .pNext = &vulkan1_2Features,
        };
        VkPhysicalDeviceFeatures2 deviceFeatures2{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &customBorderColor
        };
        vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);
        if (vulkan1_2Features.imagelessFramebuffer == VK_FALSE) {
            FatalError("Cannot init - imageless framebuffer is not supported");
        }
        if (vulkan1_3Features.dynamicRendering == VK_FALSE) {
            FatalError("Cannot init - dynamic rendering is not supported");
        }
        if (vulkan1_3Features.synchronization2 == VK_FALSE) {
            FatalError("Cannot init - synchronization2 is not supported");
        }
        if (vulkan1_2Features.scalarBlockLayout == VK_FALSE) {
            FatalError("Cannot init - ScalarBlockLayout is not supported");
        }
        if (customBorderColor.customBorderColors == VK_FALSE) {
            FatalError("Cannot init - CustomBorderColor is not supported");
        }
        if (vulkan1_2Features.samplerFilterMinmax == VK_FALSE) {
            FatalError("Cannot init - Minmax Sampler is not supported");
        }

        std::vector<const char*> runtimeExtensions{std::begin(deviceExtensions),std::end(deviceExtensions)};

#ifndef NDEBUG
        runtimeExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
#endif

        VkDeviceCreateInfo deviceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &deviceFeatures2,
            .queueCreateInfoCount = static_cast<decltype(VkDeviceCreateInfo::queueCreateInfoCount)>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),      // could pass an array here if we were making more than one queue
            .enabledExtensionCount = static_cast<uint32_t>(runtimeExtensions.size()),             // device-specific extensions are ignored on later vulkan versions but we set it anyways
            .ppEnabledExtensionNames = runtimeExtensions.data(),
            .pEnabledFeatures = nullptr,        // because we are using deviceFeatures2
        };
        if constexpr (enableValidationLayers) {
            deviceCreateInfo.enabledLayerCount = std::size(validationLayers);
            deviceCreateInfo.ppEnabledLayerNames = validationLayers;
        }
        VK_CHECK(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device));

        // load extra functions
        loadVulkanFunction(device, vkCmdPushDescriptorSetKHR, "vkCmdPushDescriptorSetKHR");
#ifndef NDEBUG
        loadVulkanFunction(device, rgl_vkDebugMarkerSetObjectNameEXT, "vkDebugMarkerSetObjectNameEXT");
        loadVulkanFunction(device, rgl_vkCmdBeginDebugUtilsLabelEXT, "vkCmdBeginDebugUtilsLabelEXT");
        loadVulkanFunction(device, rgl_vkCmdEndDebugUtilsLabelEXT, "vkCmdEndDebugUtilsLabelEXT");
#endif
        
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
        VK_VALID(presentQueue);

        VkCommandPoolCreateInfo poolInfo{
           .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
           .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,       // use this value if we want to write over the command buffer (ie for generating it every frame)
           .queueFamilyIndex = indices.graphicsFamily.value()
        };
        VK_CHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));

        VmaAllocatorCreateInfo allocInfo{
            .flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
            .physicalDevice = physicalDevice,
            .device = device,
            .preferredLargeHeapBlockSize = 0,   // default
            .pAllocationCallbacks = nullptr,
            .pDeviceMemoryCallbacks = nullptr,
            .pHeapSizeLimit = nullptr,
            .pVulkanFunctions = nullptr,
            .instance = RGL::instance,
            .vulkanApiVersion = VK_API_VERSION_1_3,
            .pTypeExternalMemoryHandleTypes = nullptr,
        };

        VK_CHECK(vmaCreateAllocator(&allocInfo,&vkallocator));
    }

    void DeviceVk::SetDebugNameForResource(void* resource, VkDebugReportObjectTypeEXT type, const char* debugName)
    {
#ifndef NDEBUG
        VkDebugMarkerObjectNameInfoEXT objectName{
           .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
           .pNext = nullptr,
           .objectType = type,
           .object = reinterpret_cast<uint64_t>(resource),
           .pObjectName = debugName
        };

        VK_CHECK(this->rgl_vkDebugMarkerSetObjectNameEXT(this->device, &objectName));
#endif
    }

    RGL::DeviceVk::~DeviceVk() {

        vmaDestroyAllocator(vkallocator);
        vkDestroyCommandPool(device, commandPool, nullptr);
        vkDestroyDevice(device, nullptr);
    }

    std::string RGL::DeviceVk::GetBrandString() {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(physicalDevice, &props);
        return props.deviceName;
    }

    RGLSwapchainPtr RGL::DeviceVk::CreateSwapchain(RGLSurfacePtr surface, RGLCommandQueuePtr, int width, int height)
    {

        return std::make_shared<SwapchainVK>(std::static_pointer_cast<RGL::SurfaceVk>(surface), shared_from_this(), width, height);
    }

    RGLPipelineLayoutPtr RGL::DeviceVk::CreatePipelineLayout(const PipelineLayoutDescriptor& pld)
    {
        return std::make_shared<PipelineLayoutVk>(shared_from_this(), pld);
    }

    RGLRenderPipelinePtr RGL::DeviceVk::CreateRenderPipeline(const RenderPipelineDescriptor& config)
    {
        return std::make_shared<RenderPipelineVk>(
            shared_from_this(),
            config);
    }

    RGLComputePipelinePtr DeviceVk::CreateComputePipeline(const ComputePipelineDescriptor& desc)
    {
        return std::make_shared<ComputePipelineVk>(shared_from_this(), desc);
    }

    RGLShaderLibraryPtr DeviceVk::CreateShaderLibraryFromName(const std::string_view& name)
    {
        FatalError("LibraryFromName not implemented");
        return RGLShaderLibraryPtr();
    }

    RGLShaderLibraryPtr RGL::DeviceVk::CreateDefaultShaderLibrary()
    {
        return std::make_shared<ShaderLibraryVk>(shared_from_this());
    }

    RGLShaderLibraryPtr RGL::DeviceVk::CreateShaderLibraryFromBytes(const std::span<const uint8_t> data)
    {
        return std::make_shared<ShaderLibraryVk>(shared_from_this(), data);
    }

    RGLShaderLibraryPtr RGL::DeviceVk::CreateShaderLibrarySourceCode(const std::string_view source, const FromSourceConfig& config)
    {
        return std::make_shared<ShaderLibraryVk>(shared_from_this(), source, config);
    }

    RGLShaderLibraryPtr RGL::DeviceVk::CreateShaderLibraryFromPath(const std::filesystem::path& path)
    {
        return std::make_shared<ShaderLibraryVk>(shared_from_this(), path);
    }

    RGLBufferPtr DeviceVk::CreateBuffer(const BufferConfig& config)
    {
        return std::make_shared<BufferVk>(shared_from_this(), config);
    }

    RGLTexturePtr DeviceVk::CreateTextureWithData(const TextureConfig& config, untyped_span bytes)
    {
        return std::make_shared<TextureVk>(shared_from_this(), config, bytes);
    }

    RGLTexturePtr DeviceVk::CreateTexture(const TextureConfig& config)
    {
        return std::make_shared<TextureVk>(shared_from_this(), config);
    }

    RGLSamplerPtr DeviceVk::CreateSampler(const SamplerConfig& config)
    {
        return std::make_shared<SamplerVk>(shared_from_this(), config);
    }

    DeviceData DeviceVk::GetDeviceData()
    {
        return {
            .vkData = {
                .device = &device,
                .physicalDevice = &physicalDevice,
                .instance = &instance,
                .queueFamilyIndex = indices.graphicsFamily.value(),
                .queueIndex = 0
            }
        };
    }

    RGLCommandQueuePtr DeviceVk::CreateCommandQueue(QueueType type)
    {
        return std::make_shared<CommandQueueVk>(shared_from_this());    // vulkan does not use the queue type
    }
    RGLFencePtr DeviceVk::CreateFence(bool preSignaled)
    {
        return std::make_shared<FenceVk>(shared_from_this(),preSignaled);
    }
    void DeviceVk::BlockUntilIdle()
    {
        vkDeviceWaitIdle(device);
    }
    size_t DeviceVk::GetTotalVRAM() const
    {
        VmaBudget budgets;
        vmaGetHeapBudgets(vkallocator, &budgets);

        return budgets.budget;
    }
    size_t DeviceVk::GetCurrentVRAMInUse() const
    {
        VmaBudget budgets;
        vmaGetHeapBudgets(vkallocator, &budgets);

        return budgets.usage;
    }
}

#endif
