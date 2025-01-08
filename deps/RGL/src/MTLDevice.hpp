#pragma once

#if RGL_MTL_AVAILABLE
#include <RGL/Types.hpp>
#include <RGL/Device.hpp>
#include "MTLObjCCompatLayer.hpp"
#include "FreeList.hpp"
#include <memory>

namespace RGL{

	struct DeviceMTL : public IDevice, public std::enable_shared_from_this<DeviceMTL>{
		OBJC_ID(MTLDevice) device = nullptr;
        OBJC_ID(MTLLibrary) defaultLibrary = nullptr;
        OBJC_ID(MTLCommandQueue) uploadQueue = nullptr;
        OBJC_ID(MTLArgumentEncoder) globalTextureEncoder = nullptr;
        OBJC_ID(MTLArgumentEncoder) globalBufferEncoder = nullptr;
        OBJC_ID(MTLBuffer) globalTextureBuffer = nullptr;
        OBJC_ID(MTLBuffer) globalBufferBuffer = nullptr;
	
        DeviceMTL(decltype(device) device);
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
        RGLTexturePtr CreateTextureWithData(const TextureConfig&, const TextureUploadData&) final;
        RGLTexturePtr CreateTexture(const TextureConfig&) final;
        RGLSamplerPtr CreateSampler(const SamplerConfig&) final;

        RGLCommandQueuePtr CreateCommandQueue(QueueType type) final;
        
        TextureView GetGlobalBindlessTextureHeap() const final;
        
        DeviceData GetDeviceData() final;

        RGLFencePtr CreateFence(bool preSignaled) final;
        void BlockUntilIdle() final;
        
        size_t GetTotalVRAM() const final;
        size_t GetCurrentVRAMInUse() const final;
        
        FreeList<uint32_t, 2048> textureFreelist;
        FreeList<uint32_t, 65536> bufferFreelist;
        
        virtual ~DeviceMTL(){}
	};

    RGLDevicePtr CreateDefaultDeviceMTL();
}

#endif
