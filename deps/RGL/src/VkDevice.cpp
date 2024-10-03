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
#include <unordered_set>
#include <vk_mem_alloc.h>

namespace RGL {

    template<typename T>
    void loadVulkanFunction(VkDevice device, T& ptr, const char* fnname, bool continueOnFail = false) {
        ptr = (std::remove_reference_t<decltype(ptr)>) vkGetDeviceProcAddr(device, fnname);
        if (!ptr && !continueOnFail) {
            FatalError(std::string("Cannot get Vulkan function pointer: ") + fnname);
        }
    }

    auto getMissingDeviceExtensions(const VkPhysicalDevice device, auto&& extensionList) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::unordered_set<std::string> requiredExtensions(std::begin(extensionList), std::end(extensionList));

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions;
    };

    constexpr static const char* const deviceExtensions[] = {
           VK_KHR_SWAPCHAIN_EXTENSION_NAME,
           VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
           VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME,
#if !__ANDROID__    // only 5% of android devices have this extension so we have to go without it
           VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
#endif
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


        // sort devices
        std::sort(devices.begin(), devices.end(), [](const VkPhysicalDevice& d1, const VkPhysicalDevice& d2) -> bool{
            // return TRUE if d1 is WORSE than d2

            // first check: discrete vs integrated -- pick discrete

            constexpr static auto GetFeatures = [](const VkPhysicalDevice& dev){
                VkPhysicalDeviceProperties vdp;
                vkGetPhysicalDeviceProperties(dev,&vdp);
                return vdp;
            };

            constexpr static auto typeRank = [](VkPhysicalDeviceType type){
                switch(type){
                    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:  return 10;
                    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return 5;
                    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return 3;
                    case VK_PHYSICAL_DEVICE_TYPE_CPU: return 2;
                    case VK_PHYSICAL_DEVICE_TYPE_OTHER: return 0;
                }
            };

            const auto features1 = GetFeatures(d1);
            const auto features2 = GetFeatures(d2);

            if (typeRank(features1.deviceType) < typeRank(features2.deviceType)){
                return true;
            }

            constexpr static auto GetTotalVRAM = [](VkPhysicalDevice dev){
                VkPhysicalDeviceMemoryProperties mem;
                vkGetPhysicalDeviceMemoryProperties(dev,&mem);

                uint64_t totalMem = 0;
                for(uint32_t i = 0; i < mem.memoryHeapCount; i++){
                    totalMem += mem.memoryHeaps[i].size;
                }
                return totalMem;
            };

            // tiebreak: we have two discrete GPUs
            // return the one with less VRAM
            if (GetTotalVRAM(d1) < GetTotalVRAM(d2)){
                return true;
            }

            // give up and say they're equivalent
            return false;
        });

        physicalDevice = devices.back();

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

        VkPhysicalDeviceVulkan13Features vulkan1_3Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = nullptr
        };
        
        VkPhysicalDeviceVulkan12Features vulkan1_2Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &vulkan1_3Features
        };

        VkPhysicalDeviceVulkan11Features vulkan1_1Features{                 // shaderDrawParameters 
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = &vulkan1_2Features
        };

        VkPhysicalDeviceCustomBorderColorFeaturesEXT customBorderColor{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT,
            .pNext = &vulkan1_1Features,
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
        if (vulkan1_2Features.scalarBlockLayout == VK_FALSE) {
            FatalError("Cannot init - ScalarBlockLayout is not supported");
        }
        if (customBorderColor.customBorderColors == VK_FALSE) {
            FatalError("Cannot init - CustomBorderColor is not supported");
        }
        if (vulkan1_2Features.samplerFilterMinmax == VK_FALSE) {
            FatalError("Cannot init - Minmax Sampler is not supported");
        }
        if (vulkan1_1Features.shaderDrawParameters == VK_FALSE) {
            FatalError("Cannot init - Shader Draw Parameters (baseInstance et al) are not supported.");
        }

        VkDeviceCreateInfo deviceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &deviceFeatures2,
            .queueCreateInfoCount = static_cast<decltype(VkDeviceCreateInfo::queueCreateInfoCount)>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),      // could pass an array here if we were making more than one queue
            .enabledExtensionCount = static_cast<uint32_t>(std::size(deviceExtensions)),             // device-specific extensions are ignored on later vulkan versions but we set it anyways
            .ppEnabledExtensionNames = deviceExtensions,
            .pEnabledFeatures = nullptr,        // because we are using deviceFeatures2
        };
        if (IsValidationEnabled()) {
            deviceCreateInfo.enabledLayerCount = std::size(validationLayers);
            deviceCreateInfo.ppEnabledLayerNames = validationLayers;
        }

        auto result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
        if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
            auto missing = getMissingDeviceExtensions(physicalDevice, deviceExtensions);
            std::string message = "vkCreateDevice error: Missing extensions:\n";
            for (const auto& ext : missing) {
                message += "\t - " + ext + "\n";
            }
            FatalError(message);
        }
        else if (result != VK_SUCCESS) {
            FatalError(std::string("vkCreateDevice failed: ") + string_VkResult(result));
        }


        //volkLoadDevice(device);
        // load extra functions
        loadVulkanFunction(device, vkCmdPushDescriptorSetKHR, "vkCmdPushDescriptorSetKHR");

#ifndef NDEBUG
        loadVulkanFunction(device, vkSetDebugUtilsObjectNameEXT, "vkSetDebugUtilsObjectNameEXT",true);
        loadVulkanFunction(device, rgl_vkCmdBeginDebugUtilsLabelEXT, "vkCmdBeginDebugUtilsLabelEXT",true);
        loadVulkanFunction(device, rgl_vkCmdEndDebugUtilsLabelEXT, "vkCmdEndDebugUtilsLabelEXT",true);
        if (vkSetDebugUtilsObjectNameEXT == nullptr || rgl_vkCmdBeginDebugUtilsLabelEXT == nullptr || rgl_vkCmdEndDebugUtilsLabelEXT == nullptr) {
            LogMessage(MessageSeverity::Warning, "Debug Utils are not present. Capture debug info will be limited.");
        }
#endif
        
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
        VK_VALID(presentQueue);

        VkCommandPoolCreateInfo poolInfo{
           .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
           .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,       // use this value if we want to write over the command buffer (ie for generating it every frame)
           .queueFamilyIndex = indices.graphicsFamily.value()
        };
        VK_CHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));

        // otherwise it doesn't know about volk
        VmaVulkanFunctions volk_functions{
            vkGetInstanceProcAddr,
            vkGetDeviceProcAddr,
            vkGetPhysicalDeviceProperties,
            vkGetPhysicalDeviceMemoryProperties,
            vkAllocateMemory,
            vkFreeMemory,
            vkMapMemory,
            vkUnmapMemory,
            vkFlushMappedMemoryRanges,
            vkInvalidateMappedMemoryRanges,
            vkBindBufferMemory,
            vkBindImageMemory,
            vkGetBufferMemoryRequirements,
            vkGetImageMemoryRequirements,
            vkCreateBuffer,
            vkDestroyBuffer,
            vkCreateImage,
            vkDestroyImage,
            vkCmdCopyBuffer,
            vkGetBufferMemoryRequirements2,
            vkGetImageMemoryRequirements2,
            vkBindBufferMemory2,
            vkBindImageMemory2,
            vkGetPhysicalDeviceMemoryProperties2,
            vkGetDeviceBufferMemoryRequirements,
            vkGetDeviceImageMemoryRequirements
        };
        
        VmaAllocatorCreateInfo allocInfo{
            .flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT | VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = physicalDevice,
            .device = device,
            .preferredLargeHeapBlockSize = 0,   // default
            .pAllocationCallbacks = nullptr,
            .pDeviceMemoryCallbacks = nullptr,
            .pHeapSizeLimit = nullptr,
            .pVulkanFunctions = &volk_functions,
            .instance = RGL::instance,
            .vulkanApiVersion = VK_API_VERSION_1_3,
            .pTypeExternalMemoryHandleTypes = nullptr,
        };

        VK_CHECK(vmaCreateAllocator(&allocInfo,&vkallocator));

        VkDescriptorBindingFlags bindingFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreate{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = 1,
            .pBindingFlags = &bindingFlags,
        };

        VkDescriptorSetLayoutBinding set_layout_binding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = nDescriptors,
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = VK_NULL_HANDLE,
        };
        VkDescriptorSetLayoutCreateInfo descriptor_layout_create_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &bindingFlagsCreate,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = 1,
            .pBindings = &set_layout_binding
        }; 
        
        VK_CHECK(vkCreateDescriptorSetLayout(device, &descriptor_layout_create_info, nullptr, &globalDescriptorSetLayout));

        // --- descriptor pool and set

        VkDescriptorPoolSize poolSizes[] = {
            {
                .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = nDescriptors
            }
        };

        VkDescriptorPoolCreateInfo poolCreate{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = 1000,        // overkill
            .poolSizeCount = std::size(poolSizes),
            .pPoolSizes = poolSizes,
        };
        VK_CHECK(vkCreateDescriptorPool(device, &poolCreate, nullptr, &globalDescriptorPool));
#if !__ANDROID__
        SetDebugNameForResource(globalDescriptorPool, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, "Bindless descriptor pool");
#endif

        VkDescriptorSetAllocateInfo setAllocInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = globalDescriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &globalDescriptorSetLayout
        };

        VK_CHECK(vkAllocateDescriptorSets(device, &setAllocInfo, &globalDescriptorSet));// we don't need to manually destroy the descriptor set
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
        if (rgl_vkSetDebugUtilsObjectNameEXT) {
            VK_CHECK(this->rgl_vkSetDebugUtilsObjectNameEXT(this->device, &objectName));
        }
#endif
    }

    RGL::DeviceVk::~DeviceVk() {

        vkDestroyDescriptorPool(device, globalDescriptorPool, VK_NULL_HANDLE);
        vkDestroyDescriptorSetLayout(device, globalDescriptorSetLayout, VK_NULL_HANDLE);
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

    TextureView DeviceVk::GetGlobalBindlessTextureHeap() const
    {
        return {
            {.bindlessSet = globalDescriptorSet}
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
        VkPhysicalDeviceMemoryProperties memprop{};
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memprop);

        stackarray(budgets, VmaBudget, memprop.memoryHeapCount);

        vmaGetHeapBudgets(vkallocator, budgets);

        size_t budget = 0;
        for (int i = 0; i < memprop.memoryHeapCount; i++) {
            budget += budgets[i].budget;
        }

        return budget;
    }
    size_t DeviceVk::GetCurrentVRAMInUse() const
    {
        VkPhysicalDeviceMemoryProperties memprop{};
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memprop);
        stackarray(budgets, VmaBudget, memprop.memoryHeapCount);

        vmaGetHeapBudgets(vkallocator, budgets);

        size_t budget = 0;
        for (int i = 0; i < memprop.memoryHeapCount; i++) {
            budget += budgets[i].usage;
        }

        return budget;
    }
}

#endif
