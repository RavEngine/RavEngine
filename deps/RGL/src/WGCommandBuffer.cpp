#if RGL_WEBGPU_AVAILABLE
#include "WGCommandBuffer.hpp"
#include "WGCommandQueue.hpp"
#include "WGRenderPass.hpp"
#include "WGBuffer.hpp"
#include "WGDevice.hpp"

namespace RGL{
    CommandBufferWG::CommandBufferWG(decltype(owningQueue) owningQueue) : owningQueue(owningQueue) { 


    }
    CommandBufferWG::~CommandBufferWG(){

    }
    
    // ICommandBuffer
    void CommandBufferWG::Reset() { 
        for(auto cb : commandBuffers){
            wgpuCommandBufferRelease(cb);
        }
        commandBuffers.clear();
    }
    void CommandBufferWG::Begin() { 
        currentCommandEncoder = wgpuDeviceCreateCommandEncoder(owningQueue->owningDevice->device, nullptr);
    }
    void CommandBufferWG::End() {
        commandBuffers.push_back(wgpuCommandEncoderFinish(currentCommandEncoder,nullptr));
        wgpuCommandEncoderRelease(currentCommandEncoder);
     }
    void CommandBufferWG::BindRenderPipeline(RGLRenderPipelinePtr) { }
    void CommandBufferWG::BeginCompute(RGLComputePipelinePtr) { }
    void CommandBufferWG::EndCompute() { }
    void CommandBufferWG::DispatchCompute(uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ, uint32_t threadsPerThreadgroupX, uint32_t threadsPerThreadgroupY, uint32_t threadsPerThreadgroupZ) { }

    void CommandBufferWG::BeginRendering(RGLRenderPassPtr pass) {
        auto casted = std::static_pointer_cast<RenderPassWG>(pass);
        currentRenderPassEncoder = wgpuCommandEncoderBeginRenderPass(currentCommandEncoder,&casted->renderPass);
    }
    void CommandBufferWG::EndRendering() { 

       wgpuRenderPassEncoderRelease(currentRenderPassEncoder);
    }

    void CommandBufferWG::BindBuffer(RGLBufferPtr buffer, uint32_t binding, uint32_t offsetIntoBuffer) { }
    void CommandBufferWG::BindComputeBuffer(RGLBufferPtr buffer, uint32_t binding, uint32_t offsetIntoBuffer) { }
    void CommandBufferWG::SetVertexBuffer(RGLBufferPtr buffer, const VertexBufferBinding& bindingInfo) {
        auto casted =std::static_pointer_cast<BufferWG>(buffer);
        wgpuRenderPassEncoderSetVertexBuffer(currentRenderPassEncoder, bindingInfo.bindingPosition, casted->buffer, bindingInfo.offsetIntoBuffer, casted->getBufferSize());

     }
            
    void CommandBufferWG::SetIndexBuffer(RGLBufferPtr buffer) { }

    void CommandBufferWG::SetVertexBytes(const untyped_span data, uint32_t offset) { }
    void CommandBufferWG::SetFragmentBytes(const untyped_span data, uint32_t offset) { }
    void CommandBufferWG::SetComputeBytes(const untyped_span data, uint32_t offset) { }
    
    void CommandBufferWG::SetVertexSampler(RGLSamplerPtr sampler, uint32_t index) { }
    void CommandBufferWG::SetFragmentSampler(RGLSamplerPtr sampler, uint32_t index) { }
    void CommandBufferWG::SetComputeSampler(RGLSamplerPtr sampler, uint32_t index) {}

    void CommandBufferWG::SetVertexTexture(const TextureView& texture, uint32_t index) { }
    void CommandBufferWG::SetFragmentTexture(const TextureView& texture, uint32_t index) { }
    void CommandBufferWG::SetComputeTexture(const TextureView& texture, uint32_t index) {}
    void CommandBufferWG::CopyTextureToTexture(const TextureCopyConfig& from, const TextureCopyConfig& to){

    }

    void CommandBufferWG::Draw(uint32_t nVertices, const DrawInstancedConfig&) { }
    void CommandBufferWG::DrawIndexed(uint32_t nIndices, const DrawIndexedInstancedConfig&) { }

    void CommandBufferWG::SetViewport(const Viewport&) { }
    void CommandBufferWG::SetScissor(const Rect&) { }
    
    void CommandBufferWG::CopyTextureToBuffer(TextureView& sourceTexture, const Rect& sourceRect, size_t offset, RGLBufferPtr desetBuffer) { }
    void CommandBufferWG::CopyBufferToBuffer(BufferCopyConfig from, BufferCopyConfig to, uint32_t size) { }

    void CommandBufferWG::CopyBufferToTexture(RGLBufferPtr source, uint32_t size, const TextureDestConfig& dest)
    {

    }

    void CommandBufferWG::UseResource(const TextureView& view){
    }


    void CommandBufferWG::Commit(const CommitConfig& config) { 
        wgpuQueueSubmit(owningQueue->queue, commandBuffers.size(), commandBuffers.data());
    }
            
    void CommandBufferWG::ExecuteIndirectIndexed(const IndirectConfig&) { }
    void CommandBufferWG::ExecuteIndirect(const IndirectConfig&) { }

    void CommandBufferWG::DispatchIndirect(const DispatchIndirectConfig&)
    {

    }
        
    void CommandBufferWG::BeginRenderDebugMarker(const std::string& label) { }
    void CommandBufferWG::BeginComputeDebugMarker(const std::string& label) { }

    void CommandBufferWG::EndRenderDebugMarker() { }
    void CommandBufferWG::EndComputeDebugMarker() { }
    void CommandBufferWG::BlockUntilCompleted(){ }
}

#endif
