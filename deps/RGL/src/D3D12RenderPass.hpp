#pragma once
#define NOMINMAX
#include <RGL/RenderPass.hpp>
#include <vector>

namespace RGL {
	struct TextureD3D12;

	struct RenderPassD3D12 : public IRenderPass {
		RenderPassConfig config;

		RenderPassD3D12(const RenderPassConfig& config);


		std::vector<TextureD3D12*> textures;

		TextureD3D12* depthTexture = nullptr;
		TextureD3D12* stencilTexture = nullptr;

		void SetAttachmentTexture(uint32_t index, ITexture* texture) final;
		void SetDepthAttachmentTexture(ITexture* texture) final;
		void SetStencilAttachmentTexture(ITexture* texture) final;
	};
}