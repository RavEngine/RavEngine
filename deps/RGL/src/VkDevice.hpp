#pragma once
#include <RGL/Types.hpp>
#include <RGL/Device.hpp>
#include <optional>
#include <memory>
#include <vulkan/vulkan.h>
#include <RGL/Pipeline.hpp>
#include <vk_mem_alloc.h>

#undef CreateSemaphore

namespace RGL {
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct DeviceVk : public IDevice, public std::enable_shared_from_this<DeviceVk> {
		VkDevice device = VK_NULL_HANDLE;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;	// does not need to be destroyed
		QueueFamilyIndices indices;
		VkQueue presentQueue = VK_NULL_HANDLE;	// do not need to be destroyed
		VkCommandPool commandPool = VK_NULL_HANDLE;
		VmaAllocator vkallocator;
		PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR;	// device-tied extension function
		PFN_vkDebugMarkerSetObjectNameEXT rgl_vkDebugMarkerSetObjectNameEXT;

		PFN_vkCmdEndDebugUtilsLabelEXT rgl_vkCmdEndDebugUtilsLabelEXT;
		PFN_vkCmdBeginDebugUtilsLabelEXT rgl_vkCmdBeginDebugUtilsLabelEXT;

		virtual ~DeviceVk();
		DeviceVk(decltype(physicalDevice) physicalDevice);

		void SetDebugNameForResource(void* resource, VkDebugReportObjectTypeEXT type, const char* debugName);

		// IDevice
		std::string GetBrandString() final;
		RGLSwapchainPtr CreateSwapchain(RGLSurfacePtr, RGLCommandQueuePtr, int width, int height) final;
		RGLPipelineLayoutPtr CreatePipelineLayout(const PipelineLayoutDescriptor&) final;
		RGLRenderPipelinePtr CreateRenderPipeline(const RenderPipelineDescriptor&) final;
		RGLComputePipelinePtr CreateComputePipeline(const ComputePipelineDescriptor&) final;

		RGLShaderLibraryPtr CreateShaderLibraryFromName(const std::string_view& name) final;
		RGLShaderLibraryPtr CreateDefaultShaderLibrary() final;
		RGLShaderLibraryPtr CreateShaderLibraryFromBytes(const std::span<uint8_t>) final;
		RGLShaderLibraryPtr CreateShaderLibrarySourceCode(const std::string_view, const FromSourceConfig& config) final;
		RGLShaderLibraryPtr CreateShaderLibraryFromPath(const std::filesystem::path&) final;

		RGLBufferPtr CreateBuffer(const BufferConfig&) final;
		RGLTexturePtr CreateTextureWithData(const TextureConfig&, untyped_span) final;
		RGLTexturePtr CreateTexture(const TextureConfig&) final;

		RGLSamplerPtr CreateSampler(const SamplerConfig&) final;

		DeviceData GetDeviceData() final;

		RGLCommandQueuePtr CreateCommandQueue(QueueType type) final;
		RGLFencePtr CreateFence(bool preSignaled) final;
		void BlockUntilIdle() final;
	};

	RGLDevicePtr CreateDefaultDeviceVk();
}