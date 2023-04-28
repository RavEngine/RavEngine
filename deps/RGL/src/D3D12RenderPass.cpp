#if RGL_DX12_AVAILABLE
#include "D3D12RenderPass.hpp"
#include "D3D12Texture.hpp"

namespace RGL {
	RenderPassD3D12::RenderPassD3D12(const RenderPassConfig& config) : config(config), textures(config.attachments.size(), nullptr)
	{
	}
	void RenderPassD3D12::SetAttachmentTexture(uint32_t index, ITexture* texture)
	{
		textures.at(index) = static_cast<TextureD3D12*>(texture);
	}
	void RenderPassD3D12::SetDepthAttachmentTexture(ITexture* texture)
	{
		depthTexture = static_cast<TextureD3D12*>(texture);
	}
	void RenderPassD3D12::SetStencilAttachmentTexture(ITexture* texture)
	{
		stencilTexture = static_cast<TextureD3D12*>(texture);
	}
}
#endif