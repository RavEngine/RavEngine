#pragma once
#include <RGL/Types.hpp>
#include <RGL/RenderPass.hpp>
#include "VkDevice.hpp"
#include <RGL/Texture.hpp>

namespace RGL {
	struct TextureVk;

	struct RenderPassVk : public IRenderPass {
		RenderPassConfig config;

		RenderPassVk(const RenderPassConfig&);
		virtual ~RenderPassVk();

		std::vector<TextureView> textures;

		std::optional<TextureView> depthTexture;
		std::optional<TextureView> stencilTexture;

		void SetAttachmentTexture(uint32_t index, const TextureView& texture) final;
		void SetDepthAttachmentTexture(const TextureView& texture) final;
		void SetStencilAttachmentTexture(const TextureView& texture) final;
	};

}