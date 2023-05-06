#if RGL_MTL_AVAILABLE
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include "MTLDevice.hpp"
#include "MTLSurface.hpp"
#include "MTLSwapchain.hpp"
#include "RGLCommon.hpp"
#include "MTLSynchronization.hpp"
#include "MTLShaderLibrary.hpp"
#include "MTLBuffer.hpp"
#include "MTLPipeline.hpp"
#include "MTLCommandQueue.hpp"
#include "MTLTexture.hpp"
#include "MTLSampler.hpp"
#include "MTLRenderPass.hpp"
#include "MTLComputePipeline.hpp"

namespace RGL{
RGLDevicePtr CreateDefaultDeviceMTL(){
    auto device = MTLCreateSystemDefaultDevice();
    return std::make_shared<DeviceMTL>(device);
}

DeviceMTL::DeviceMTL(decltype(device) device)  : device(device){
    defaultLibrary = [device newDefaultLibrary];
    uploadQueue = [device newCommandQueue];
}

std::string DeviceMTL::GetBrandString() {
    
    auto name = [device name];
    return std::string([name UTF8String]);
}

RGLSwapchainPtr DeviceMTL::CreateSwapchain(RGLSurfacePtr isurface, RGLCommandQueuePtr presentQueue, int width, int height){
    auto surface = std::static_pointer_cast<RGL::SurfaceMTL>(isurface);
    [surface->layer setDevice:device];
    return std::make_shared<SwapchainMTL>(surface, width, height);
}

RGLPipelineLayoutPtr DeviceMTL::CreatePipelineLayout(const PipelineLayoutDescriptor& desc) {
    return std::make_shared<PipelineLayoutMTL>(desc);
}

RGLRenderPipelinePtr DeviceMTL::CreateRenderPipeline(const RenderPipelineDescriptor& desc) {
    return std::make_shared<RenderPipelineMTL>(shared_from_this(), desc);
}

RGLComputePipelinePtr DeviceMTL::CreateComputePipeline(const RGL::ComputePipelineDescriptor& desc) {
    return std::make_shared<ComputePipelineMTL>(shared_from_this(), desc);
}

RGLShaderLibraryPtr DeviceMTL::CreateDefaultShaderLibrary() {
    FatalError("CreateDefaultShaderLibrary not implemented");
}

RGLShaderLibraryPtr DeviceMTL::CreateShaderLibraryFromName(const std::string_view& name){
    return std::make_shared<ShaderLibraryMTL>(shared_from_this(), name);
}

RGLShaderLibraryPtr DeviceMTL::CreateShaderLibraryFromBytes(const std::span<uint8_t>) {
    FatalError("Not Implemented");
}

RGLShaderLibraryPtr DeviceMTL::CreateShaderLibrarySourceCode(const std::string_view source, const FromSourceConfig& config) {
    return std::make_shared<ShaderLibraryMTL>(shared_from_this(), source, config);
}

RGLShaderLibraryPtr DeviceMTL::CreateShaderLibraryFromPath(const std::filesystem::path&) {
    FatalError("ShaderLibraryMTL");
}

RGLBufferPtr DeviceMTL::CreateBuffer(const BufferConfig& config) {
    return std::make_shared<BufferMTL>(shared_from_this(), config);
}

RGLCommandQueuePtr DeviceMTL::CreateCommandQueue(QueueType type) {
    return std::make_shared<CommandQueueMTL>(shared_from_this());
}

RGLFencePtr DeviceMTL::CreateFence(bool preSignaled) {
    return std::make_shared<FenceMTL>();
}

RGLTexturePtr DeviceMTL::CreateTextureWithData(const TextureConfig& config, untyped_span data){
    return std::make_shared<TextureMTL>(shared_from_this(), config, data);
}

RGLTexturePtr DeviceMTL::CreateTexture(const TextureConfig& config){
    return std::make_shared<TextureMTL>(shared_from_this(), config);
}

RGLSamplerPtr DeviceMTL::CreateSampler(const SamplerConfig& config){
    return std::make_shared<SamplerMTL>(shared_from_this(), config);
}

void DeviceMTL::BlockUntilIdle() {
    
}

RGL::DeviceData DeviceMTL::GetDeviceData() {
    return {
        .mtlData{
            .device = (void*)device
        }
    };
}

}

#endif





