#pragma once

#if RGL_MTL_AVAILABLE
#include <RGL/Types.hpp>
#include <RGL/Device.hpp>
#include "MTLObjCCompatLayer.hpp"
#include <memory>

namespace RGL{

	struct DeviceMTL : public IDevice, public std::enable_shared_from_this<DeviceMTL>{
		OBJC_ID(MTLDevice) device = nullptr;
        OBJC_ID(MTLLibrary) defaultLibrary = nullptr;
	
        DeviceMTL(decltype(device) device);
		std::string GetBrandString() final;
		
        RGLSwapchainPtr CreateSwapchain(RGLSurfacePtr, RGLCommandQueuePtr presentQueue, int, int) final;

        RGLPipelineLayoutPtr CreatePipelineLayout(const PipelineLayoutDescriptor&) final;
        RGLRenderPipelinePtr CreateRenderPipeline(const RenderPipelineDescriptor&) final;
        RGLComputePipelinePtr CreateComputePipeline(const struct ComputePipelineDescriptor&) final;

        RGLShaderLibraryPtr CreateShaderLibraryFromName(const std::string_view& name) final;
        RGLShaderLibraryPtr CreateDefaultShaderLibrary() final;
        RGLShaderLibraryPtr CreateShaderLibraryFromBytes(const std::span<uint8_t>) final;
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
        
        virtual ~DeviceMTL(){}
	};

    RGLDevicePtr CreateDefaultDeviceMTL();
}

#endif
