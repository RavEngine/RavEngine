#pragma once
#include <RGL/RenderPass.hpp>
#include <vector>
#include <RGL/Texture.hpp>

namespace RGL {
	struct TextureD3D12;

	struct RenderPassD3D12 : public IRenderPass {
		RenderPassConfig config;

		RenderPassD3D12(const RenderPassConfig& config);


		std::vector<TextureView> textures;

		std::optional<TextureView> depthTexture;
		std::optional <TextureView> stencilTexture;

		void SetAttachmentTexture(uint32_t index, const TextureView& texture) final;
		void SetDepthAttachmentTexture(const TextureView& texture) final;
		void SetStencilAttachmentTexture(const TextureView& texture) final;
	};
}
