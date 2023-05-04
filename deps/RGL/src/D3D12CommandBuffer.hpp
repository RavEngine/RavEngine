#pragma once
#define NOMINMAX
#include <RGL/Types.hpp>
#include <RGL/CommandBuffer.hpp>
#include "RGLD3D12.hpp"
#include <d3d12.h>
#include <memory>

namespace RGL {
	struct CommandQueueD3D12;
	struct TextureD3D12;

	struct CommandBufferD3D12 : public ICommandBuffer {
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList;
		const std::shared_ptr<CommandQueueD3D12> owningQueue;

		std::shared_ptr<struct RenderPassD3D12> currentRenderPass;
		std::shared_ptr<struct RenderPipelineD3D12> currentRenderPipeline;
		std::shared_ptr<struct ComputePipelineD3D12> currentComputePipeline;

		bool ended = false;

		CommandBufferD3D12(decltype(owningQueue));

		// clear the command buffer, to encode new commands
		void Reset() final;

		// signal that new commands are arriving
		void Begin() final;

		// signal that all commands ahve been encoded
		void End() final;

		void BeginRendering(RGLRenderPassPtr) final;
		void EndRendering() final;

		void BindRenderPipeline(RGLRenderPipelinePtr) final;
		void BeginCompute(RGLComputePipelinePtr) final;
		void EndCompute() final;
		void DispatchCompute(uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ) final;

		void BindBuffer(RGLBufferPtr buffer, uint32_t bindingOffset, uint32_t offsetIntoBuffer = 0) final;
		void BindComputeBuffer(RGLBufferPtr buffer, uint32_t binding, uint32_t offsetIntoBuffer = 0) final;
		void SetVertexBuffer(RGLBufferPtr buffer, const VertexBufferBinding& bindingInfo = {}) final;

		void SetVertexBytes(const untyped_span data, uint32_t offset) final;
		void SetFragmentBytes(const untyped_span data, uint32_t offset) final;
		void SetComputeBytes(const untyped_span data, uint32_t offset) final;

		void SetIndexBuffer(RGLBufferPtr buffer) final;

		void SetVertexSampler(RGLSamplerPtr sampler, uint32_t index) final;
		void SetFragmentSampler(RGLSamplerPtr sampler, uint32_t index) final;

		void SetVertexTexture(const ITexture* texture, uint32_t index) final;
		void SetFragmentTexture(const ITexture* texture, uint32_t index) final;

		void SetCombinedTextureSampler(RGLSamplerPtr sampler, const ITexture* texture, uint32_t index) final;

		void Draw(uint32_t nVertices, const DrawInstancedConfig & = {}) final;
		void DrawIndexed(uint32_t nIndices, const DrawIndexedInstancedConfig & = {}) final;

		void SetViewport(const Viewport&) final;
		void SetScissor(const Rect&) final;

		void SetRenderPipelineBarrier(const BarrierConfig&) final;

		void CopyTextureToBuffer(RGL::ITexture* sourceTexture, const Rect& sourceRect, size_t offset, RGLBufferPtr desetBuffer) final;

		void TransitionResource(const ITexture* texture, RGL::ResourceLayout current, RGL::ResourceLayout target, TransitionPosition position) final;

		// submit onto the queue that created this command buffer
		void Commit(const CommitConfig&) final;

		void ExecuteIndirectIndexed(const IndirectConfig&) final;
		void ExecuteIndirect(const IndirectConfig&) final;

		~CommandBufferD3D12() {}
	};
}
