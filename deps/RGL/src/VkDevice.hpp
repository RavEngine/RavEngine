#pragma once
#include <RGL/Types.hpp>
#include <RGL/Device.hpp>
#include <optional>
#include <memory>
#include <volk.h>
#include <RGL/Pipeline.hpp>
#include "FreeList.hpp"

#undef CreateSemaphore

struct VmaAllocator_T;

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
        VmaAllocator_T* vkallocator;
		PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR = nullptr;	// device-tied extension function

		PFN_vkDebugMarkerSetObjectNameEXT rgl_vkSetDebugUtilsObjectNameEXT = nullptr;
		PFN_vkCmdEndDebugUtilsLabelEXT rgl_vkCmdEndDebugUtilsLabelEXT = nullptr;
		PFN_vkCmdBeginDebugUtilsLabelEXT rgl_vkCmdBeginDebugUtilsLabelEXT = nullptr;

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
		RGLShaderLibraryPtr CreateShaderLibraryFromBytes(const std::span<const uint8_t>) final;
		RGLShaderLibraryPtr CreateShaderLibrarySourceCode(const std::string_view, const FromSourceConfig& config) final;
		RGLShaderLibraryPtr CreateShaderLibraryFromPath(const std::filesystem::path&) final;

		RGLBufferPtr CreateBuffer(const BufferConfig&) final;
		RGLTexturePtr CreateTextureWithData(const TextureConfig&, untyped_span) final;
		RGLTexturePtr CreateTexture(const TextureConfig&) final;

		RGLSamplerPtr CreateSampler(const SamplerConfig&) final;

		DeviceData GetDeviceData() final;

		TextureView GetGlobalBindlessTextureHeap() const final;

		RGLCommandQueuePtr CreateCommandQueue(QueueType type) final;
		RGLFencePtr CreateFence(bool preSignaled) final;
		void BlockUntilIdle() final;

		size_t GetTotalVRAM() const final;
		size_t GetCurrentVRAMInUse() const final;

		uint32_t frameIndex = 0;

		VkDescriptorSetLayout globalDescriptorSetLayout = VK_NULL_HANDLE;

		constexpr static uint32_t nDescriptors = 2048;		       // made-up number (matches the DX backend)
		FreeList<uint32_t, nDescriptors> globalDescriptorFreeList;

		VkDescriptorSet globalDescriptorSet = VK_NULL_HANDLE;

	private:
		VkDeviceSize globalDescriptorSetOffset = 0;
		void* globalDescriptorMappedMemory = nullptr;

		VkDescriptorPool globalDescriptorPool = VK_NULL_HANDLE;
	};

	RGLDevicePtr CreateDefaultDeviceVk();
}
