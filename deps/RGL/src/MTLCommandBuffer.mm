#if RGL_MTL_AVAILABLE
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include "MTLCommandBuffer.hpp"
#include "MTLCommandQueue.hpp"
#include "MTLPipeline.hpp"
#include "MTLTexture.hpp"
#include "MTLBuffer.hpp"
#include "MTLSampler.hpp"
#include "MTLRenderPass.hpp"
#include "MTLDevice.hpp"
#include "MTLComputePipeline.hpp"
#include "RGLCommon.hpp"
#include <librglc.hpp>

namespace RGL{

MTLWinding rgl2mtlwinding(RGL::WindingOrder order){
    switch (order){
        case decltype(order)::Clockwise: return MTLWindingClockwise;
        case decltype(order)::Counterclockwise: return MTLWindingCounterClockwise;
    }
}

MTLCullMode rgl2mtlcullmode(RGL::CullMode mode){
    switch (mode){
        case decltype(mode)::None: return MTLCullModeNone;
        case decltype(mode)::Front: return MTLCullModeFront;
        case decltype(mode)::Back: return MTLCullModeBack;
        case decltype(mode)::Both:
        default:
            FatalError("Invalid cullmode");
    }
}

uint32_t bytesPerPixel(MTLPixelFormat format){
    switch (format){
        case MTLPixelFormatInvalid:
            return 0;
        case MTLPixelFormatA8Unorm:
        case MTLPixelFormatR8Unorm:
        case MTLPixelFormatR8Snorm:
        case MTLPixelFormatR8Uint:
        case MTLPixelFormatR8Sint:
        case MTLPixelFormatR8Unorm_sRGB:
        case MTLPixelFormatStencil8:
            return 1;
        case MTLPixelFormatR16Unorm:
        case MTLPixelFormatR16Snorm:
        case MTLPixelFormatR16Uint:
        case MTLPixelFormatR16Sint:
        case MTLPixelFormatR16Float:
            return 2;
        case MTLPixelFormatRG8Unorm:
        case MTLPixelFormatRG8Unorm_sRGB:
        case MTLPixelFormatRG8Snorm:
        case MTLPixelFormatRG8Uint:
        case MTLPixelFormatRG8Sint:
            return 1 * 2;
        case MTLPixelFormatR32Uint:
        case MTLPixelFormatR32Sint:
        case MTLPixelFormatR32Float:
        case MTLPixelFormatDepth32Float:
            return 4;
        case MTLPixelFormatRG16Unorm:
        case MTLPixelFormatRG16Snorm:
        case MTLPixelFormatRG16Uint:
        case MTLPixelFormatRG16Sint:
        case MTLPixelFormatRG16Float:
            return 2 * 2;
        case MTLPixelFormatRGBA8Unorm:
        case MTLPixelFormatRGBA8Unorm_sRGB:
        case MTLPixelFormatRGBA8Snorm:
        case MTLPixelFormatRGBA8Uint:
        case MTLPixelFormatRGBA8Sint:
        case MTLPixelFormatBGRA8Unorm_sRGB:
            return 1 * 4;
        case MTLPixelFormatRG32Uint:
        case MTLPixelFormatRG32Sint:
        case MTLPixelFormatRG32Float:
            return 4 * 2;
        case MTLPixelFormatRGBA16Unorm:
        case MTLPixelFormatRGBA16Snorm:
        case MTLPixelFormatRGBA16Uint:
        case MTLPixelFormatRGBA16Sint:
        case MTLPixelFormatRGBA16Float:
            return 2 * 4;
        case MTLPixelFormatRGBA32Uint:
        case MTLPixelFormatRGBA32Sint:
        case MTLPixelFormatRGBA32Float:
            return 4 * 4;
        case MTLPixelFormatDepth16Unorm:
            return 2;
#if !TARGET_OS_IPHONE
        case MTLPixelFormatDepth24Unorm_Stencil8:
#endif
        case MTLPixelFormatDepth32Float_Stencil8:
            return 4;
        default:
            FatalError("Unsupported pixel format");
    }
}

CommandBufferMTL::CommandBufferMTL(decltype(owningQueue) owningQueue) : owningQueue(owningQueue){
    auto dummydepthdesc = [MTLDepthStencilDescriptor new];
    noDepthStencil = [owningQueue->owningDevice->device newDepthStencilStateWithDescriptor:dummydepthdesc];
}

void CommandBufferMTL::Reset(){
    currentCommandEncoder = nullptr;
    currentCommandBuffer = nullptr;
    vertexBuffer = nullptr;
    indexBuffer = nullptr;
}

void CommandBufferMTL::Begin(){
    currentCommandBuffer = [owningQueue->commandQueue commandBuffer];
}

void CommandBufferMTL::End(){
    
}

void CommandBufferMTL::SetIndexBuffer(RGLBufferPtr buffer) {
    indexBuffer = std::static_pointer_cast<BufferMTL>(buffer);
}

void CommandBufferMTL::BindRenderPipeline(RGLRenderPipelinePtr pipelineIn){
    auto pipeline = std::static_pointer_cast<RenderPipelineMTL>(pipelineIn);
    [currentCommandEncoder setRenderPipelineState: pipeline->pipelineState];
    if (pipeline->depthStencilState){
        [currentCommandEncoder setDepthStencilState:pipeline->depthStencilState];
    }
    else{
        [currentCommandEncoder setDepthStencilState:noDepthStencil];
    }
    [currentCommandEncoder setFrontFacingWinding:rgl2mtlwinding(pipeline->settings.rasterizerConfig.windingOrder)];
    [currentCommandEncoder setCullMode:rgl2mtlcullmode(pipeline->settings.rasterizerConfig.cullMode)];
    [currentCommandEncoder setTriangleFillMode:pipeline->currentFillMode];
}

void CommandBufferMTL::BeginCompute(RGLComputePipelinePtr pipelineIn){
    currentComputeCommandEncoder = [currentCommandBuffer computeCommandEncoder];
    [currentComputeCommandEncoder setComputePipelineState:std::static_pointer_cast<ComputePipelineMTL>(pipelineIn)->pipelineState];
}

void CommandBufferMTL::EndCompute(){
    [currentComputeCommandEncoder endEncoding];
}

void CommandBufferMTL::DispatchCompute(uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ, uint32_t threadsPerThreadgroupX, uint32_t threadsPerThreadgroupY, uint32_t threadsPerThreadgroupZ){
    [currentComputeCommandEncoder dispatchThreadgroups:MTLSizeMake(threadsX, threadsY, threadsZ) threadsPerThreadgroup:MTLSizeMake(threadsPerThreadgroupX, threadsPerThreadgroupY, threadsPerThreadgroupZ)];
}

void CommandBufferMTL::BeginRendering(RGLRenderPassPtr renderPass){
    auto casted = std::static_pointer_cast<RenderPassMTL>(renderPass);
    currentCommandEncoder = [currentCommandBuffer renderCommandEncoderWithDescriptor:casted->renderPassDescriptor];
}

void CommandBufferMTL::EndRendering(){
    [currentCommandEncoder endEncoding];
}

void CommandBufferMTL::BindBuffer(RGLBufferPtr buffer, uint32_t binding, uint32_t offsetIntoBuffer){
    //TODO: something smarter than this
    [currentCommandEncoder setVertexBuffer:std::static_pointer_cast<BufferMTL>(buffer)->buffer offset:offsetIntoBuffer atIndex:binding];
    [currentCommandEncoder setFragmentBuffer:std::static_pointer_cast<BufferMTL>(buffer)->buffer offset:offsetIntoBuffer atIndex:binding];
}

void CommandBufferMTL::BindComputeBuffer(RGLBufferPtr buffer, uint32_t binding, uint32_t offsetIntoBuffer){
    [currentComputeCommandEncoder setBuffer:std::static_pointer_cast<BufferMTL>(buffer)->buffer offset:offsetIntoBuffer atIndex:binding];
}

void CommandBufferMTL::SetVertexBuffer(RGLBufferPtr buffer, const VertexBufferBinding& bindingInfo) {
    vertexBuffer = std::static_pointer_cast<BufferMTL>(buffer);
    [currentCommandEncoder setVertexBuffer:vertexBuffer->buffer offset:bindingInfo.offsetIntoBuffer * vertexBuffer->stride atIndex:bindingInfo.bindingPosition];
}

void CommandBufferMTL::SetVertexBytes(const untyped_span data, uint32_t offset){
    [currentCommandEncoder setVertexBytes: data.data() length:data.size() atIndex: offset+librglc::MTL_FIRST_BUFFER];
}

void CommandBufferMTL::SetComputeBytes(const untyped_span data, uint32_t offset){
    [currentComputeCommandEncoder setBytes:data.data() length:data.size() atIndex:offset+librglc::MTL_FIRST_BUFFER];
}

void CommandBufferMTL::SetFragmentBytes(const untyped_span data, uint32_t offset){
    [currentCommandEncoder setFragmentBytes: data.data() length:data.size() atIndex: offset+librglc::MTL_FIRST_BUFFER];

}

void CommandBufferMTL::Draw(uint32_t nVertices, const DrawInstancedConfig& config){
    [currentCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:config.startVertex * vertexBuffer->stride vertexCount:nVertices instanceCount:config.nInstances baseInstance:config.firstInstance];
}

void CommandBufferMTL::DrawIndexed(uint32_t nIndices, const DrawIndexedInstancedConfig& config){
    assert(indexBuffer != nil); // did you forget to call SetIndexBuffer?
    auto indexType = MTLIndexTypeUInt32;
    if (indexBuffer->stride == 2){
        indexType = MTLIndexTypeUInt16;
    }
    
    [currentCommandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:nIndices indexType:indexType indexBuffer:indexBuffer->buffer indexBufferOffset:config.firstIndex * indexBuffer->stride instanceCount:config.nInstances baseVertex:config.startVertex * vertexBuffer->stride baseInstance:config.firstInstance];
}

void CommandBufferMTL::SetViewport(const Viewport & viewport){
    MTLViewport vp{
        .originX = viewport.x,
        .originY = viewport.y,
        .width = viewport.width,
        .height = viewport.height,
        .znear = viewport.minDepth,
        .zfar = viewport.maxDepth
    };
    
    [currentCommandEncoder setViewport:vp];
}

void CommandBufferMTL::SetScissor(const Rect & scissor){
    MTLScissorRect sr{
        .x = static_cast<NSUInteger>(scissor.offset[0]),
        .y = static_cast<NSUInteger>(scissor.offset[1]),
        .width = scissor.extent[0],
        .height = scissor.extent[1]
    };
    
    [currentCommandEncoder setScissorRect:sr];
}

void CommandBufferMTL::Commit(const CommitConfig & config){
    [currentCommandBuffer commit];
    [currentCommandBuffer waitUntilCompleted]; // TODO: delete this
}

void CommandBufferMTL::SetVertexSampler(RGLSamplerPtr sampler, uint32_t index) {
    [currentCommandEncoder setVertexSamplerState:std::static_pointer_cast<SamplerMTL>(sampler)->sampler atIndex:index];
}
void CommandBufferMTL::SetFragmentSampler(RGLSamplerPtr sampler, uint32_t index) {
    [currentCommandEncoder setFragmentSamplerState:std::static_pointer_cast<SamplerMTL>(sampler)->sampler atIndex:index];
}

void CommandBufferMTL::SetVertexTexture(const ITexture* texture, uint32_t index){
    [currentCommandEncoder setVertexTexture:static_cast<const TextureMTL*>(texture)->texture atIndex:index];
}
void CommandBufferMTL::SetFragmentTexture(const ITexture* texture, uint32_t index){
    [currentCommandEncoder setFragmentTexture:static_cast<const TextureMTL*>(texture)->texture atIndex:index];
}

void CommandBufferMTL::CopyTextureToBuffer(RGL::ITexture *sourceTexture, const RGL::Rect &sourceRect, size_t offset, RGLBufferPtr destBuffer) {
    auto blitencoder = [currentCommandBuffer blitCommandEncoder];
    auto castedTexture = static_cast<TextureMTL*>(sourceTexture);
    auto castedBuffer = std::static_pointer_cast<BufferMTL>(destBuffer);
    
    auto bytesPerRow = bytesPerPixel([castedTexture->texture pixelFormat]);
    bytesPerRow *= castedTexture->GetSize().width;
    
    [blitencoder copyFromTexture:castedTexture->texture
                     sourceSlice:0
                     sourceLevel:0
                    sourceOrigin:MTLOriginMake(sourceRect.offset[0], sourceRect.offset[1], 0)
                      sourceSize:MTLSizeMake(sourceRect.extent[0], sourceRect.extent[1], 1)
                        toBuffer:castedBuffer->buffer
               destinationOffset:0
          destinationBytesPerRow:bytesPerRow
        destinationBytesPerImage:0];

    [blitencoder endEncoding];
}

void CommandBufferMTL::SetResourceBarrier(const ResourceBarrierConfig& config){
#if 0
    auto size = config.buffers.size() + config.textures.size();
    stackarray(resources, id<MTLResource>, size);
    
    uint32_t i = 0;
    for(const auto& bufferBase : config.buffers){
        resources[i] = std::static_pointer_cast<BufferMTL>(bufferBase)->buffer;
    }
    i = config.buffers.size();
    for(const auto& textureBase : config.textures){
        resources[i] = std::static_pointer_cast<TextureMTL>(textureBase)->texture;
    }
    
    constexpr auto stages = MTLRenderStageVertex | MTLRenderStageFragment;
    [currentCommandEncoder memoryBarrierWithResources:resources count:size afterStages:stages beforeStages:stages];
#endif
}

void CommandBufferMTL::SetRenderPipelineBarrier(const PipelineBarrierConfig&) {

}

void CommandBufferMTL::CopyBufferToBuffer(BufferCopyConfig from, BufferCopyConfig to, uint32_t size){
    auto blitEncoder = [currentCommandBuffer blitCommandEncoder];
    auto fromBuffer = std::static_pointer_cast<BufferMTL>(from.buffer);
    auto toBuffer = std::static_pointer_cast<BufferMTL>(to.buffer);
    [blitEncoder copyFromBuffer:fromBuffer->buffer sourceOffset:from.offset toBuffer:toBuffer->buffer destinationOffset:to.offset size:size];
    [blitEncoder endEncoding];
}

void CommandBufferMTL::TransitionResource(const ITexture* texture, RGL::ResourceLayout current, RGL::ResourceLayout target, TransitionPosition position) {
    // no effect on Metal
}
void CommandBufferMTL::TransitionResources(std::initializer_list<ResourceTransition> transitions, TransitionPosition position)
{
    // no effect on Metal
}

void CommandBufferMTL::BeginRenderDebugMarker(const std::string &label){
    [currentCommandEncoder pushDebugGroup:[NSString stringWithUTF8String:label.c_str()]];
}

void CommandBufferMTL::BeginComputeDebugMarker(const std::string &label){
    [currentComputeCommandEncoder pushDebugGroup:[NSString stringWithUTF8String:label.c_str()]];
}

void CommandBufferMTL::EndRenderDebugMarker(){
    [currentCommandEncoder popDebugGroup];
}

void CommandBufferMTL::EndComputeDebugMarker(){
    [currentComputeCommandEncoder popDebugGroup];
}

void CommandBufferMTL::ExecuteIndirectIndexed(const RGL::IndirectConfig & config) {
    auto buffer = std::static_pointer_cast<BufferMTL>(config.indirectBuffer);
    assert(indexBuffer != nil); // did you forget to call SetIndexBuffer?
    auto indexType = MTLIndexTypeUInt32;
    if (indexBuffer->stride == 2){
        indexType = MTLIndexTypeUInt16;
    }
    
    // because Metal doesn't have multidraw...
    for(uint32_t i = 0; i < config.nDraws; i++){
        [currentCommandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexType:indexType indexBuffer:indexBuffer->buffer indexBufferOffset:0 indirectBuffer:buffer->buffer indirectBufferOffset:config.offsetIntoBuffer + i * sizeof(IndirectIndexedCommand)];
    }
    
}

void CommandBufferMTL::ExecuteIndirect(const RGL::IndirectConfig & config) {
    auto buffer = std::static_pointer_cast<BufferMTL>(config.indirectBuffer);
    
    // because Metal doesn't have multidraw...
    for(uint32_t i = 0; i < config.nDraws; i++){
        [currentCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangle indirectBuffer:buffer->buffer indirectBufferOffset:config.offsetIntoBuffer + i * sizeof(IndirectCommand)];
    }
}

}
#endif


