#pragma once
#include <RGL/Types.hpp>
#include <RGL/Device.hpp>
#include <span>
#include <emscripten/html5_webgpu.h>

namespace RGL{
    struct DeviceWG : public IDevice, public std::enable_shared_from_this<DeviceWG>{
        WGPUDevice device;
        WGPUAdapter adapter;

        DeviceWG();
		std::string GetBrandString() final;
		
        RGLSwapchainPtr CreateSwapchain(RGLSurfacePtr, RGLCommandQueuePtr presentQueue, int, int) final;

        RGLPipelineLayoutPtr CreatePipelineLayout(const PipelineLayoutDescriptor&) final;
        RGLRenderPipelinePtr CreateRenderPipeline(const RenderPipelineDescriptor&) final;
        RGLComputePipelinePtr CreateComputePipeline(const struct ComputePipelineDescriptor&) final;

        RGLShaderLibraryPtr CreateShaderLibraryFromName(const std::string_view& name) final;
        RGLShaderLibraryPtr CreateDefaultShaderLibrary() final;
        RGLShaderLibraryPtr CreateShaderLibraryFromBytes(const std::span<const uint8_t>) final;
        RGLShaderLibraryPtr CreateShaderLibrarySourceCode(const std::string_view, const FromSourceConfig& config) final;
        RGLShaderLibraryPtr CreateShaderLibraryFromPath(const std::filesystem::path&) final;

        RGLBufferPtr CreateBuffer(const BufferConfig&) final;
        RGLTexturePtr CreateTextureWithData(const TextureConfig&, untyped_span) final;
        RGLTexturePtr CreateTexture(const TextureConfig&) final;
        RGLSamplerPtr CreateSampler(const SamplerConfig&) final;

        RGLCommandQueuePtr CreateCommandQueue(QueueType type) final;
        
        DeviceData GetDeviceData() final;

        RGLFencePtr CreateFence(bool preSignaled) final;
        void BlockUntilIdle() final;
        
        virtual ~DeviceWG();
    };

    RGLDevicePtr CreateDefaultDeviceWG();
}