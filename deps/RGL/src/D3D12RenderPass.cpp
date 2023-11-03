#if RGL_DX12_AVAILABLE
#include "D3D12RenderPass.hpp"
#include "D3D12Texture.hpp"

namespace RGL {
	RenderPassD3D12::RenderPassD3D12(const RenderPassConfig& config) : config(config), textures(config.attachments.size())
	{
	}
	void RenderPassD3D12::SetAttachmentTexture(uint32_t index, const TextureView& texture)
	{
		textures.at(index) = texture;
	}
	void RenderPassD3D12::SetDepthAttachmentTexture(const TextureView& texture)
	{
		depthTexture = texture;
	}
	void RenderPassD3D12::SetStencilAttachmentTexture(const TextureView& texture)
	{
		stencilTexture = texture;
	}
}
#endif