#pragma once
#include <RGL/Types.hpp>
#include <RGL/CommandBuffer.hpp>
#include "RGLD3D12.hpp"
#include "D3D12TrackedResource.hpp"
#include <directx/d3d12.h>
#include <memory>
#include <unordered_map>

namespace RGL {
	struct D3D12TextureLastUseKey {
		const struct D3D12TrackedResource* texture = nullptr;
		covered_mips_t coveredMips;
		covered_layers_t coveredLayers;
		D3D12TextureLastUseKey(decltype(texture) texture, decltype(coveredMips) coveredMips, decltype(coveredLayers) coveredLayers) : texture(texture), coveredMips(coveredMips), coveredLayers(coveredLayers) {}
		bool operator==(const D3D12TextureLastUseKey& other) const {
			return texture == other.texture && coveredMips == other.coveredMips && coveredLayers == other.coveredLayers;
		}
	};
}

namespace std {
	template<>
	struct hash<RGL::D3D12TextureLastUseKey> {
		size_t operator()(const RGL::D3D12TextureLastUseKey& other) const {
			return uintptr_t(other.texture) ^ other.coveredMips ^ other.coveredLayers;
		}
	};
}

namespace RGL {
	struct CommandQueueD3D12;
	struct TextureD3D12;
	struct BufferD3D12;

	struct CommandBufferD3D12 : public ICommandBuffer {
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList;
		const std::shared_ptr<CommandQueueD3D12> owningQueue;

		std::shared_ptr<struct RenderPassD3D12> currentRenderPass;
		std::shared_ptr<struct RenderPipelineD3D12> currentRenderPipeline;
		std::shared_ptr<struct ComputePipelineD3D12> currentComputePipeline;

		//TODO: also store if the last use was a write or a read
		//if a read, then we need to insert a standard barrier even if it's already in the right state
		struct ResourceLastUse {
			D3D12_RESOURCE_STATES state;
			bool written = false;
		};
		std::unordered_map<const struct D3D12TrackedResource*, ResourceLastUse> activeBuffers;

		std::unordered_map<D3D12TextureLastUseKey, ResourceLastUse> activeTextures;

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
		void DispatchCompute(uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ, uint32_t threadsPerThreadgroupX=1, uint32_t threadsPerThreadgroupY=1, uint32_t threadsPerThreadgroupZ=1) final;

		void BindBuffer(RGLBufferPtr buffer, uint32_t bindingOffset, uint32_t offsetIntoBuffer = 0) final;
		void BindComputeBuffer(RGLBufferPtr buffer, uint32_t binding, uint32_t offsetIntoBuffer = 0) final;
		void SetVertexBuffer(RGLBufferPtr buffer, const VertexBufferBinding& bindingInfo = {}) final;

		void SetVertexBytes(const untyped_span data, uint32_t offset) final;
		void SetFragmentBytes(const untyped_span data, uint32_t offset) final;
		void SetComputeBytes(const untyped_span data, uint32_t offset) final;

		void SetIndexBuffer(RGLBufferPtr buffer) final;

		void SetVertexSampler(RGLSamplerPtr sampler, uint32_t index) final;
		void SetFragmentSampler(RGLSamplerPtr sampler, uint32_t index) final;
		void SetComputeSampler(RGLSamplerPtr sampler, uint32_t index) final;

		void SetVertexTexture(const TextureView& texture, uint32_t index) final;
		void SetFragmentTexture(const TextureView& texture, uint32_t index) final;
		void SetComputeTexture(const TextureView& texture, uint32_t index) final;

		void Draw(uint32_t nVertices, const DrawInstancedConfig & = {}) final;
		void DrawIndexed(uint32_t nIndices, const DrawIndexedInstancedConfig & = {}) final;

		void BindBindlessBufferDescriptorSet(uint32_t set_idx) final;

		void SetViewport(const Viewport&) final;
		void SetScissor(const Rect&) final;

		void CopyTextureToBuffer(TextureView& sourceTexture, const Rect& sourceRect, size_t offset, RGLBufferPtr desetBuffer) final;

		void CopyBufferToTexture(RGLBufferPtr source, uint32_t size, const TextureDestConfig& dest) final;

		void CopyBufferToBuffer(BufferCopyConfig from, BufferCopyConfig to, uint32_t size) final;

		void CopyTextureToTexture(const TextureCopyConfig& from, const TextureCopyConfig& to) final;

		// submit onto the queue that created this command buffer
		void Commit(const CommitConfig&) final;

		void ExecuteIndirectIndexed(const IndirectConfig&) final;
		void ExecuteIndirect(const IndirectConfig&) final;
		void DispatchIndirect(const DispatchIndirectConfig&) final;

		void BeginRenderDebugMarker(const std::string& label) final;
		void BeginComputeDebugMarker(const std::string& label) final;

		void EndRenderDebugMarker() final;
		void EndComputeDebugMarker() final;
        
        void UseResource(const TextureView& tx) final;
		void UseResource(const RGLBufferPtr buffer) final;

		void BlockUntilCompleted() final;

		virtual ~CommandBufferD3D12();
	private:
		void SyncIfNeeded(const BufferD3D12* buffer, D3D12_RESOURCE_STATES needed, bool written = false);
		void SyncIfNeeded(TextureView texture, D3D12_RESOURCE_STATES needed, bool written);
		D3D12_RESOURCE_STATES GetBufferCurrentResourceState(const struct D3D12TrackedResource*);
		HANDLE internalFenceEvent;
		Microsoft::WRL::ComPtr<ID3D12Fence> internalFence;
	};
}
