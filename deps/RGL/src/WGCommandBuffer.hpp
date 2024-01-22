#pragma once
#include <RGL/CommandBuffer.hpp>
#include <emscripten/html5_webgpu.h>

namespace RGL{
    struct CommandQueueWG;

    struct CommandBufferWG : public ICommandBuffer{

        const std::shared_ptr<CommandQueueWG> owningQueue;
        CommandBufferWG(decltype(owningQueue));
        
        // ICommandBuffer
        void Reset() final;
        void Begin() final;
        void End() final;
        void BindRenderPipeline(RGLRenderPipelinePtr) final;
        void BeginCompute(RGLComputePipelinePtr) final;
        void EndCompute() final;
        void DispatchCompute(uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ, uint32_t threadsPerThreadgroupX=1, uint32_t threadsPerThreadgroupY=1, uint32_t threadsPerThreadgroupZ=1) final;

        void BeginRendering(RGLRenderPassPtr) final;
        void EndRendering() final;

        void BindBuffer(RGLBufferPtr buffer, uint32_t binding, uint32_t offsetIntoBuffer = 0) final;
        void BindComputeBuffer(RGLBufferPtr buffer, uint32_t binding, uint32_t offsetIntoBuffer = 0) final;
        void SetVertexBuffer(RGLBufferPtr buffer, const VertexBufferBinding& bindingInfo = {}) final;
                
        void SetIndexBuffer(RGLBufferPtr buffer) final;

        void SetVertexBytes(const untyped_span data, uint32_t offset) final;
        void SetFragmentBytes(const untyped_span data, uint32_t offset) final;
        void SetComputeBytes(const untyped_span data, uint32_t offset) final;
        
        void SetVertexSampler(RGLSamplerPtr sampler, uint32_t index) final;
        void SetFragmentSampler(RGLSamplerPtr sampler, uint32_t index) final;
        void SetComputeSampler(RGLSamplerPtr sampler, uint32_t index) final;
        
        void SetVertexTexture(const TextureView& texture, uint32_t index) final;
        void SetFragmentTexture(const TextureView& texture, uint32_t index) final;
        void SetComputeTexture(const TextureView& texture, uint32_t index) final;

        void CopyTextureToTexture(const TextureCopyConfig& from, const TextureCopyConfig& to) final;

        void Draw(uint32_t nVertices, const DrawInstancedConfig& = {}) final;
        void DrawIndexed(uint32_t nIndices, const DrawIndexedInstancedConfig& = {}) final;

        void SetViewport(const Viewport&) final;
        void SetScissor(const Rect&) final;
        
        void CopyTextureToBuffer(TextureView& sourceTexture, const Rect& sourceRect, size_t offset, RGLBufferPtr desetBuffer) final;
        void CopyBufferToBuffer(BufferCopyConfig from, BufferCopyConfig to, uint32_t size) final;
        void CopyBufferToTexture(RGLBufferPtr source, uint32_t size, const TextureDestConfig& dest) final;

        void Commit(const CommitConfig&) final;
                
        void ExecuteIndirectIndexed(const IndirectConfig&) final;
        void ExecuteIndirect(const IndirectConfig&) final;
        
        virtual ~CommandBufferWG();
        
        void BeginRenderDebugMarker(const std::string& label) final;
        void BeginComputeDebugMarker(const std::string& label) final;

        void EndRenderDebugMarker() final;
        void EndComputeDebugMarker() final;


        void BlockUntilCompleted() final;

    private:
        WGPUCommandEncoder currentCommandEncoder;
        WGPURenderPassEncoder currentRenderPassEncoder;
        std::vector<WGPUCommandBuffer> commandBuffers;
    };
}