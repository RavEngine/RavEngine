#pragma once
#include <memory>

#define RGL_NBACKENDS (RGL_MTL_AVAILABLE + RGL_VK_AVAILABLE + RGL_DX12_AVAILABLE)
#define RGL_SINGLE_BACKEND RGL_NBACKENDS == 1

namespace RGL {
	struct ISwapchain;
	struct IRenderPass;
	struct ISurface;
	struct IPipelineLayout;
	struct IRenderPipeline;

	struct RenderPassConfig;
	struct RenderPipelineDescriptor;
	struct PipelineLayoutDescriptor;
	struct IShaderLibrary;

	struct BufferConfig;
	struct IBuffer;
	struct IFence;
	struct ITexture;
	struct TextureConfig;
	struct ISampler;
	struct SamplerConfig;
	struct IDevice;
	struct ICommandBuffer;
	struct ICommandQueue;
	struct ISwapchain;
    struct IRenderPass;
	struct IComputePipeline;
	struct ICustomTextureView;
}

#if RGL_SINGLE_BACKEND
#if RGL_MTL_AVAILABLE
namespace RGL {
    struct SwapchainMTL;
    struct RenderPassMTL;
    struct SurfaceMTL;
    struct PipelineLayoutMTL;
    struct RenderPipelineMTL;

    struct ShaderLibraryMTL;

    struct BufferMTL;
    struct FenceMTL;
    struct SemaphoreMTL;
    struct TextureMTL;
    struct SamplerMTL;
    struct DeviceMTL;
    struct CommandQueueMTL;
    struct CommandBufferMTL;
    struct RenderPassMTL;
    struct ComputePipelineMTL;
	struct CustomTextureViewMTL;
}

using RGLDevicePtr = std::shared_ptr<RGL::DeviceMTL>;
using RGLSwapchainPtr = std::shared_ptr<RGL::SwapchainMTL>;
using RGLRenderPassPtr = std::shared_ptr<RGL::RenderPassMTL>;
using RGLSurfacePtr = std::shared_ptr<RGL::SurfaceMTL>;
using RGLPipelineLayoutPtr = std::shared_ptr<RGL::PipelineLayoutMTL>;
using RGLRenderPipelinePtr = std::shared_ptr<RGL::RenderPipelineMTL>;
using RGLShaderLibraryPtr = std::shared_ptr<RGL::ShaderLibraryMTL>;
using RGLBufferPtr = std::shared_ptr<RGL::BufferMTL>;
using RGLFencePtr = std::shared_ptr<RGL::FenceMTL>;
using RGLTexturePtr = std::shared_ptr<RGL::TextureMTL>;
using RGLSamplerPtr = std::shared_ptr<RGL::SamplerMTL>;
using RGLCommandQueuePtr = std::shared_ptr<RGL::CommandQueueMTL>;
using RGLCommandBufferPtr = std::shared_ptr<RGL::CommandBufferMTL>;
using RGLRenderPassPtr = std::shared_ptr<RGL::RenderPassMTL>;
using RGLComputePipelinePtr = std::shared_ptr<RGL::ComputePipelineMTL>;
using RGLCustomTextureViewPtr = std::shared_ptr<RGL::CustomTextureViewMTL>;


#elif RGL_VK_AVAILABLE
namespace RGL {
	struct SwapchainVK;
	struct RenderPassVk;
	struct SurfaceVk;
	struct PipelineLayoutVk;
	struct RenderPipelineVk;

	struct ShaderLibraryVk;

	struct BufferVk;
	struct FenceVk;
	struct SemaphoreVk;
	struct TextureVk;
	struct SamplerVk;
	struct DeviceVk;
	struct CommandQueueVk;
	struct CommandBufferVk;
	struct ComputePipelineVk;
	struct CustomTextureViewVk;
}

using RGLDevicePtr = std::shared_ptr<RGL::DeviceVk>;
using RGLSwapchainPtr = std::shared_ptr<RGL::SwapchainVK>;
using RGLRenderPassPtr = std::shared_ptr<RGL::RenderPassVk>;
using RGLSurfacePtr = std::shared_ptr<RGL::SurfaceVk>;
using RGLPipelineLayoutPtr = std::shared_ptr<RGL::PipelineLayoutVk>;
using RGLRenderPipelinePtr = std::shared_ptr<RGL::RenderPipelineVk>;
using RGLShaderLibraryPtr = std::shared_ptr<RGL::ShaderLibraryVk>;
using RGLBufferPtr = std::shared_ptr<RGL::BufferVk>;
using RGLFencePtr = std::shared_ptr<RGL::FenceVk>;
using RGLTexturePtr = std::shared_ptr<RGL::TextureVk>;
using RGLSamplerPtr = std::shared_ptr<RGL::SamplerVk>;
using RGLCommandQueuePtr = std::shared_ptr<RGL::CommandQueueVk>;
using RGLCommandBufferPtr = std::shared_ptr<RGL::CommandBufferVk>;
using RGLComputePipelinePtr = std::shared_ptr<RGL::ComputePipelineVk>;
using RGLCustomTextureViewPtr = std::shared_ptr<RGL::CustomTextureViewVk>;

#elif RGL_DX12_AVAILABLE
namespace RGL {
	struct SwapchainD3D12;
	struct RenderPassD3D12;
	struct SurfaceD3D12;
	struct PipelineLayoutD3D12;
	struct RenderPipelineD3D12;

	struct ShaderLibraryD3D12;

	struct BufferD3D12;
	struct FenceD3D12;
	struct SemaphoreD3D12;
	struct TextureD3D12;
	struct SamplerD3D12;
	struct DeviceD3D12;
	struct CommandQueueD3D12;
	struct CommandBufferD3D12;
	struct ComputePipelineD3D12;
	struct CustomTextureViewD3D12;
}

using RGLDevicePtr = std::shared_ptr<RGL::DeviceD3D12>;
using RGLSwapchainPtr = std::shared_ptr<RGL::SwapchainD3D12>;
using RGLRenderPassPtr = std::shared_ptr<RGL::RenderPassD3D12>;
using RGLSurfacePtr = std::shared_ptr<RGL::SurfaceD3D12>;
using RGLPipelineLayoutPtr = std::shared_ptr<RGL::PipelineLayoutD3D12>;
using RGLRenderPipelinePtr = std::shared_ptr<RGL::RenderPipelineD3D12>;
using RGLShaderLibraryPtr = std::shared_ptr<RGL::ShaderLibraryD3D12>;
using RGLBufferPtr = std::shared_ptr<RGL::BufferD3D12>;
using RGLFencePtr = std::shared_ptr<RGL::FenceD3D12>;
using RGLTexturePtr = std::shared_ptr<RGL::TextureD3D12>;
using RGLSamplerPtr = std::shared_ptr<RGL::SamplerD3D12>;
using RGLCommandQueuePtr = std::shared_ptr<RGL::CommandQueueD3D12>;
using RGLCommandBufferPtr = std::shared_ptr<RGL::CommandBufferD3D12>;
using RGLRenderPassPtr = std::shared_ptr<RGL::RenderPassD3D12>;
using RGLComputePipelinePtr = std::shared_ptr<RGL::ComputePipelineD3D12>;
using RGLCustomTextureViewPtr = std::shared_ptr<RGL::CustomTextureViewD3D12>;
#endif

#else
	using RGLDevicePtr = std::shared_ptr<RGL::IDevice>;
	using RGLRenderPassPtr = std::shared_ptr<RGL::IRenderPass>;
	using RGLSurfacePtr = std::shared_ptr<RGL::ISurface>;
	using RGLPipelineLayoutPtr = std::shared_ptr<RGL::IPipelineLayout>;
	using RGLRenderPipelinePtr = std::shared_ptr<RGL::IRenderPipeline>;
	using RGLShaderLibraryPtr = std::shared_ptr<RGL::IShaderLibrary>;
	using RGLBufferPtr = std::shared_ptr<RGL::IBuffer>;
	using RGLFencePtr = std::shared_ptr<RGL::IFence>;
	using RGLTexturePtr = std::shared_ptr<RGL::ITexture>;
	using RGLCustomTextureViewPtr = std::shared_ptr<RGL::ICustomTextureView>;
	using RGLSamplerPtr = std::shared_ptr<RGL::ISampler>;
	using RGLCommandQueuePtr = std::shared_ptr<RGL::ICommandQueue>;
	using RGLCommandBufferPtr = std::shared_ptr<RGL::ICommandBuffer>;
	using RGLSwapchainPtr = std::shared_ptr<RGL::ISwapchain>;
    using RGLRenderPassPtr = std::shared_ptr<RGL::IRenderPass>;
	using RGLComputePipelinePtr = std::shared_ptr<RGL::IComputePipeline>;
#endif

