#pragma once
#include <RGL/Types.hpp>
#include <RGL/CommandBuffer.hpp>
#include "MTLObjCCompatLayer.hpp"
#include <array>

namespace RGL{
struct CommandQueueMTL;
struct TextureMTL;
struct IBuffer;
struct BufferMTL;

    struct CommandBufferMTL : public ICommandBuffer{
        OBJC_ID(MTLCommandBuffer) currentCommandBuffer = nullptr;
        OBJC_ID(MTLRenderCommandEncoder) currentCommandEncoder = nullptr;
        OBJC_ID(MTLComputeCommandEncoder) currentComputeCommandEncoder = nullptr;
        OBJC_ID(MTLDepthStencilState) noDepthStencil = nullptr;
        
        std::shared_ptr<BufferMTL> indexBuffer;
        std::shared_ptr<BufferMTL> vertexBuffer;
        
        const std::shared_ptr<CommandQueueMTL> owningQueue;
        CommandBufferMTL(decltype(owningQueue));
        
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
        
        void SetVertexTexture(const TextureView& texture, uint32_t index) final;
        void SetFragmentTexture(const TextureView& texture, uint32_t index) final;
        void SetComputeTexture(const TextureView& texture, uint32_t index) final;

        void Draw(uint32_t nVertices, const DrawInstancedConfig& = {}) final;
        void DrawIndexed(uint32_t nIndices, const DrawIndexedInstancedConfig& = {}) final;

        void SetViewport(const Viewport&) final;
        void SetScissor(const Rect&) final;
        
        void CopyTextureToBuffer(RGL::TextureView& sourceTexture, const Rect& sourceRect, size_t offset, RGLBufferPtr desetBuffer) final;
        void CopyBufferToBuffer(BufferCopyConfig from, BufferCopyConfig to, uint32_t size) final;

        void Commit(const CommitConfig&) final;
                
        virtual void ExecuteIndirectIndexed(const IndirectConfig&) final;
        virtual void ExecuteIndirect(const IndirectConfig&) final;
        
        virtual ~CommandBufferMTL(){}
        
        void BeginRenderDebugMarker(const std::string& label) final;
        void BeginComputeDebugMarker(const std::string& label) final;

        void EndRenderDebugMarker() final;
        void EndComputeDebugMarker() final;
    };

}
