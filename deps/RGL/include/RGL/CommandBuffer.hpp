#pragma once
#include <RGL/Types.hpp>
#include <memory>
#include <array>
#include <RGL/Texture.hpp>
#include <RGL/Buffer.hpp>
#include <RGL/Synchronization.hpp>
#include <span>
#include <string>

struct DrawInstancedConfig{
    uint32_t
    nInstances = 1,
    startVertex = 0,
    firstInstance = 0;
};

struct DrawIndexedInstancedConfig{
    uint32_t
    nInstances = 1,
    firstIndex = 0,
    firstInstance = 0,
    startVertex = 0;
};

namespace RGL {
	struct IRenderPipeline;
    struct ISampler;
    struct ITexture;

	struct Viewport {
		float 
			x = 0, 
			y = 0,
			width = 1, 
			height = 1, 
			minDepth = 0, 
			maxDepth = 1;
	};
	struct Rect {
		std::array<int32_t, 2> offset{ 0,0 };
		std::array<uint32_t, 2> extent{ 0,0, };
	};


	struct CommitConfig {
		RGLFencePtr signalFence;
	};

	struct ResourceBarrierConfig {
		std::initializer_list<RGLBufferPtr> buffers;
		std::initializer_list<RGLTexturePtr> textures;
	};

	struct IndirectIndexedCommand {
		uint32_t indexCount;
		uint32_t instanceCount;
		uint32_t indexStart;
		uint32_t baseVertex;
		uint32_t baseInstance;
	};

	struct IndirectCommand {
		uint32_t vertexCount;
		uint32_t instanceCount;
		uint32_t firstVertex;
		uint32_t firstInstance;
	};

	struct IndirectConfig {
		RGLBufferPtr indirectBuffer;
		uint32_t offsetIntoBuffer = 0;	// in bytes
		uint32_t nDraws;
	};

	struct VertexBufferBinding {
		uint32_t bindingPosition = 0;
		uint32_t offsetIntoBuffer = 0;
	};

	struct PipelineBarrierConfig {
		bool Vertex : 1 = false;
		bool Fragment : 1 = false;
		bool Compute : 1 = false;
	};

struct TextureView;

	struct ICommandBuffer {
		// clear the command buffer, to encode new commands
		virtual void Reset() = 0;

		// signal that new commands are arriving
		virtual void Begin() = 0;

		// signal that all commands ahve been encoded
		virtual void End() = 0;

		virtual void BeginRendering(RGLRenderPassPtr) = 0;
		virtual void EndRendering() = 0;
        

		virtual void BindRenderPipeline(RGLRenderPipelinePtr) = 0;
        virtual void BeginCompute(RGLComputePipelinePtr) = 0;
        virtual void EndCompute() = 0;
        virtual void DispatchCompute(uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ, uint32_t threadsPerThreadgroupX=1, uint32_t threadsPerThreadgroupY=1, uint32_t threadsPerThreadgroupZ=1) = 0;

		virtual void BindBuffer(RGLBufferPtr buffer, uint32_t binding, uint32_t offsetIntoBuffer = 0) = 0;
		virtual void BindComputeBuffer(RGLBufferPtr buffer, uint32_t binding, uint32_t offsetIntoBuffer = 0) = 0;
        
		virtual void SetVertexBuffer(RGLBufferPtr buffer, const VertexBufferBinding& bindingInfo = {}) = 0;

        virtual void SetIndexBuffer(RGLBufferPtr buffer) = 0;
        
        virtual void SetVertexSampler(RGLSamplerPtr sampler, uint32_t index) = 0;
        virtual void SetFragmentSampler(RGLSamplerPtr sampler, uint32_t index) = 0;
        virtual void SetComputeSampler(RGLSamplerPtr sampler, uint32_t index) = 0;
        
        virtual void SetVertexTexture(const TextureView& texture, uint32_t index) = 0;
        virtual void SetFragmentTexture(const TextureView& texture, uint32_t index) = 0;
        virtual void SetComputeTexture(const TextureView& texture, uint32_t index) = 0;
        
        virtual void Draw(uint32_t nVertices, const DrawInstancedConfig& = {}) = 0;
        virtual void DrawIndexed(uint32_t nIndices, const DrawIndexedInstancedConfig& = {}) = 0;

		virtual void SetViewport(const Viewport&) = 0;
		virtual void SetScissor(const Rect&) = 0;

		virtual void CopyTextureToBuffer(TextureView& sourceTexture, const Rect& sourceRect, size_t offset, RGLBufferPtr desetBuffer) = 0;

		struct TextureDestConfig {
			TextureView view;
			Rect destLoc;
			uint32_t arrayLayer = 0;
		};
		virtual void CopyBufferToTexture(RGLBufferPtr source, uint32_t size, const TextureDestConfig& dest) = 0;

		struct BufferCopyConfig {
			RGLBufferPtr buffer;
			uint32_t offset = 0;
		};
		virtual void CopyBufferToBuffer(BufferCopyConfig from, BufferCopyConfig to, uint32_t size) = 0;
        
        struct TextureCopyConfig{
            TextureView texture;
        };
        virtual void CopyTextureToTexture(const TextureCopyConfig& from, const TextureCopyConfig& to) = 0;

		// submit onto the queue that created this command buffer
		virtual void Commit(const CommitConfig&) = 0;

		virtual void SetVertexBytes(const untyped_span data, uint32_t offset) = 0;
		virtual void SetFragmentBytes(const untyped_span data, uint32_t offset) = 0;
		virtual void SetComputeBytes(const untyped_span data, uint32_t offset) = 0;

		virtual void ExecuteIndirectIndexed(const IndirectConfig&) = 0;
		virtual void ExecuteIndirect(const IndirectConfig&) = 0;

		virtual void BeginRenderDebugMarker(const std::string& label) = 0;
		virtual void BeginComputeDebugMarker(const std::string& label) = 0;

		virtual void EndRenderDebugMarker() = 0;
		virtual void EndComputeDebugMarker() = 0;

		virtual void BlockUntilCompleted() = 0;
	};
}
