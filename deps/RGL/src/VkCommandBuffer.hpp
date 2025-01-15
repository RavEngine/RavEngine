#pragma once
#include <RGL/Types.hpp>
#include <RGL/CommandBuffer.hpp>
#include <volk.h>
#include "RGLVk.hpp"
#include <unordered_set>
#include <unordered_map>
#include <RGL/SubresourceRange.hpp>

namespace RGL {
	struct TextureLastUseKey {
		const struct TextureVk* texture = nullptr;
		covered_mips_t coveredMips;
		covered_layers_t coveredLayers;
		TextureLastUseKey(decltype(texture) texture, decltype(coveredMips) coveredMips, decltype(coveredLayers) coveredLayers) : texture(texture), coveredMips(coveredMips), coveredLayers(coveredLayers) {}

		bool operator==(const TextureLastUseKey& other) const {
			return texture == other.texture && coveredMips == other.coveredMips && coveredLayers == other.coveredLayers;
		}
	};
}

namespace std {
	template<>
	struct hash<RGL::TextureLastUseKey> {
		size_t operator()(const RGL::TextureLastUseKey& other) const {
			return uintptr_t(other.texture) ^ other.coveredMips ^ other.coveredLayers;
		}
	};
}

namespace RGL {
	struct DeviceVk;
	struct CommandQueueVk;
	struct RenderPipelineVk;
	struct RenderPassVk;

	struct CommandBufferVk : public ICommandBuffer {
		bool isInsideRenderingBlock = false;
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;	// does not need to be destroyed
		std::shared_ptr<RenderPassVk> currentRenderPass = nullptr;

		const std::shared_ptr<CommandQueueVk> owningQueue;
		std::shared_ptr<RenderPipelineVk> currentRenderPipeline = nullptr;
		std::shared_ptr<struct ComputePipelineVk> currentComputePipeline = nullptr;

		std::unordered_set<struct SwapchainVK*> swapchainsToSignal;
		std::unordered_set<const struct TextureVk*> swapchainImages;

		struct TextureLastUse {
			VkImageLayout lastLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			bool written = false;
		};

		struct BufferLastUse {
			bool written = false;
		};


		bool keyIsAllMips(const TextureLastUseKey& key) {
			return key.coveredMips == ALL_MIPS;
		}

		std::unordered_map<TextureLastUseKey, TextureLastUse> activeTextures;
		std::unordered_map<const struct BufferVk*, BufferLastUse> activeBuffers;

		struct CmdSetVertexBuffer {
			RGLBufferPtr buffer;
			const VertexBufferBinding bindingInfo;
		};
		struct CmdSetIndexBuffer {
			RGLBufferPtr buffer;
		};

		struct CmdBindRenderPipeline {
			RGLRenderPipelinePtr generic_pipeline;
		};

		struct CmdBindBuffer {
			RGLBufferPtr buffer;
			const uint32_t offsetIntoBuffer;
			const uint32_t bindingOffset;
			VkPipelineBindPoint bindPoint;
		};

		struct CmdSetPushConstantData {
			std::byte data[256]{ };
			uint32_t size = 0;
			uint32_t offset = 0;
		};

		struct CmdSetSampler {
			RGLSamplerPtr sampler;
			uint32_t index;
			bool isCompute = false;
		};

		struct CmdSetTexture {
			TextureView texture;
			uint32_t index;
		};

		struct CmdDraw {
			uint32_t nVertices;
			const DrawInstancedConfig config;
		};

		struct CmdDrawIndexed {
			uint32_t nIndices;
			const DrawIndexedInstancedConfig config;
		};

		struct CmdExecuteIndirect {
			const IndirectConfig config;
		};

		struct CmdExecuteIndirectIndexed {
			const IndirectConfig config;
		};

		struct CmdDispatchIndirect {
			const DispatchIndirectConfig config;
		};

		struct CmdBeginDebugMarker {
			const std::string label;
		};
		struct CmdEndDebugMarker {};

		struct CmdBeginRendering {
			RGLRenderPassPtr pass;
		};

		struct CmdBeginCompute {
			RGLComputePipelinePtr inPipeline;
		};

		struct CmdEndCompute {};

		struct CmdDispatch {
			uint32_t threadsX;
			uint32_t threadsY;
			uint32_t threadsZ;
		};

		struct CmdSetViewport {
			const Viewport viewport;
		};

		struct CmdSetScissor {
			const Rect scissor;
		};

		struct CmdCopyTextureToBuffer {
			TextureView sourceTexture;
			const Rect sourceRect;
			size_t offset;
			RGLBufferPtr destBuffer;
		};

		struct CmdCopyBuffertoTexture {
			RGLBufferPtr srcBuffer;
			uint32_t nBytes;
			TextureView destTexture;
			Rect destLoc;
			uint32_t arrayLayer;
		};

		struct CmdCopyBufferToBuffer {
			BufferCopyConfig from;
			BufferCopyConfig to;
			uint32_t size;
		};

		struct CmdBindlessSetTexture {
			const VkDescriptorSet set;
			const uint32_t binding;
		};

		struct CmdBindlessSetBuffer {
			const VkDescriptorSet set;
			const uint32_t setIndex;
		};

		struct CmdCopyTextureToTexture {
			TextureCopyConfig from, to;
			uint32_t fromMip, fromLayer, toMip, toLayer;
		};

		std::vector < std::variant <
			CmdSetVertexBuffer,
			CmdBeginRendering,
			CmdSetIndexBuffer,
			CmdSetSampler,
			CmdSetTexture,
			CmdDraw,
			CmdDrawIndexed,
			CmdBindBuffer,
			CmdExecuteIndirect,
			CmdExecuteIndirectIndexed,
			CmdDispatchIndirect,
			CmdSetPushConstantData,
			CmdBindRenderPipeline,
			CmdBeginDebugMarker,
			CmdEndDebugMarker,
			CmdBeginCompute,
			CmdEndCompute,
			CmdDispatch,
			CmdCopyTextureToBuffer,
			CmdCopyTextureToTexture,
			CmdSetViewport,
			CmdSetScissor,
			CmdCopyBufferToBuffer,
			CmdCopyBuffertoTexture,
			CmdBindlessSetTexture,
			CmdBindlessSetBuffer
		>
		> renderCommands;

		void EncodeCommand(auto&& commandValue) {
			renderCommands.push_back(commandValue);
			if (!isInsideRenderingBlock) {
				ApplyBarriers();
				EncodeQueuedCommands();				// run it immediately if not inside a beginrendering
			}
		}

		CommandBufferVk(decltype(owningQueue) owningQueue);

		~CommandBufferVk();

		// ICommandBuffer
		void Reset() final;
		void Begin() final;
		void End() final;
		void BindRenderPipeline(RGLRenderPipelinePtr) final;

		void BeginRendering(RGLRenderPassPtr) final;
		void EndRendering() final;

		void BeginCompute(RGLComputePipelinePtr) final;
		void EndCompute() final;
		void DispatchCompute(uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ, uint32_t threadsPerThreadgroupX = 1, uint32_t threadsPerThreadgroupY = 1, uint32_t threadsPerThreadgroupZ = 1) final;

		void BindBuffer(RGLBufferPtr buffer, uint32_t bindingOffset, uint32_t offsetIntoBuffer = 0) final;

		void BindComputeBuffer(RGLBufferPtr buffer, uint32_t binding, uint32_t offsetIntoBuffer = 0) final;
		void SetVertexBuffer(RGLBufferPtr buffer, const VertexBufferBinding& bindingInfo = {}) final;

		void setPushConstantData(const RGL::untyped_span& data, const uint32_t& offset);

		void SetVertexBytes(const untyped_span data, uint32_t offset) final;
		void SetFragmentBytes(const untyped_span data, uint32_t offset) final;
		void SetComputeBytes(const untyped_span data, uint32_t offset) final;

		void SetIndexBuffer(RGLBufferPtr buffer) final;

		void SetVertexSampler(RGLSamplerPtr sampler, uint32_t index) final;
		void SetFragmentSampler(RGLSamplerPtr sampler, uint32_t index) final;
		void SetComputeSampler(RGLSamplerPtr sampler, uint32_t index) final;

		void BindBindlessBufferDescriptorSet(uint32_t set_idx) final;

		void SetVertexTexture(const TextureView& texture, uint32_t index) final;
		void SetFragmentTexture(const TextureView& texture, uint32_t index) final;
		void SetComputeTexture(const TextureView& texture, uint32_t index) final;

		void Draw(uint32_t nVertices, const DrawInstancedConfig & = {}) final;
		void DrawIndexed(uint32_t nIndices, const DrawIndexedInstancedConfig & = {}) final;

		void CopyTextureToBuffer(TextureView& sourceTexture, const Rect& sourceRect, size_t offset, RGLBufferPtr destBuffer) final;
		void CopyBufferToTexture(RGLBufferPtr source, uint32_t size, const TextureDestConfig& dest) final;
		void CopyBufferToBuffer(BufferCopyConfig from, BufferCopyConfig to, uint32_t size) final;
		void CopyTextureToTexture(const TextureCopyConfig& from, const TextureCopyConfig& to) final;

		void SetViewport(const Viewport&) final;
		void SetScissor(const Rect&) final;
        
        void UseResource(const TextureView& tx) final;

		void Commit(const CommitConfig&) final;

		void ExecuteIndirectIndexed(const IndirectConfig&) final;
		void ExecuteIndirect(const IndirectConfig&) final;
		void DispatchIndirect(const DispatchIndirectConfig&) final;

		void BeginRenderDebugMarker(const std::string& label) final;
		void BeginComputeDebugMarker(const std::string& label) final;

		void EndRenderDebugMarker() final;
		void EndComputeDebugMarker() final;

		void BlockUntilCompleted() final;

	private:
		void GenericBindBuffer(RGLBufferPtr& buffer, const uint32_t& offsetIntoBuffer, const uint32_t& bindingOffset, VkPipelineBindPoint bindPoint);
		void RecordBufferBinding(const BufferVk* buffer, BufferLastUse usage);
		void RecordTextureBinding(const TextureView texture, TextureLastUse usage, bool recordOnly = false);
		void EncodeQueuedCommands();
		bool IsBufferSlotWritable(uint32_t slot);
		std::vector<VkBufferMemoryBarrier2> barriersToAdd;
		void ApplyBarriers();
		VkFence internalFence;
	};
}

