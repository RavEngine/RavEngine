#pragma once
#include <memory>
#include <string>
#include <span>
#include <filesystem>
#include <RGL/CommandQueue.hpp>
#include <RGL/Span.hpp>
#include <RGL/Types.hpp>
#include <RGL/ShaderLibrary.hpp>

#undef CreateSemaphore

struct ID3D12Device;

namespace RGL {

	union DeviceData {
		struct {
			ID3D12Device* device;
		} d3d12Data;
		struct {
			void* device;
			void* physicalDevice;
			void* instance;
			uint32_t queueFamilyIndex, queueIndex;
		} vkData;
        struct {
            void* device;
        } mtlData;
	};

	struct TextureView;
	struct TextureUploadData;

	struct IDevice {
		virtual ~IDevice() {}
		static RGLDevicePtr CreateSystemDefaultDevice();

		virtual std::string GetBrandString() = 0;
		
		virtual RGLSwapchainPtr CreateSwapchain(RGLSurfacePtr, RGLCommandQueuePtr presentQueue, int width, int height) = 0;

		virtual RGLPipelineLayoutPtr CreatePipelineLayout(const PipelineLayoutDescriptor&) = 0;
		virtual RGLRenderPipelinePtr CreateRenderPipeline(const RenderPipelineDescriptor&) = 0;

        virtual RGLShaderLibraryPtr CreateShaderLibraryFromName(const std::string_view& name) = 0;
		virtual RGLShaderLibraryPtr CreateDefaultShaderLibrary() = 0;
		virtual RGLShaderLibraryPtr CreateShaderLibraryFromBytes(const std::span<const uint8_t>) = 0;
		virtual RGLShaderLibraryPtr CreateShaderLibrarySourceCode(const std::string_view, const FromSourceConfig& config) = 0;
		virtual RGLShaderLibraryPtr CreateShaderLibraryFromPath(const std::filesystem::path&) = 0;

		virtual RGLBufferPtr CreateBuffer(const BufferConfig&) = 0;


		virtual RGLTexturePtr CreateTextureWithData(const TextureConfig&, const TextureUploadData&) = 0;
		virtual RGLTexturePtr CreateTexture(const TextureConfig&) = 0;
        virtual RGLSamplerPtr CreateSampler(const SamplerConfig&) = 0;

		virtual RGLCommandQueuePtr CreateCommandQueue(QueueType type) = 0;

		virtual TextureView GetGlobalBindlessTextureHeap() const = 0;

		virtual RGLComputePipelinePtr CreateComputePipeline(const struct ComputePipelineDescriptor&) = 0;

		virtual size_t GetTotalVRAM() const = 0;
		virtual size_t GetCurrentVRAMInUse() const = 0;

		virtual DeviceData GetDeviceData() = 0;

		virtual RGLFencePtr CreateFence(bool preSignaled) = 0;
		virtual void BlockUntilIdle() = 0;
	};
}
