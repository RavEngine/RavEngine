#if RGL_VK_AVAILABLE
#include "VkRenderPass.hpp"
#include "RGLVk.hpp"
#include "VkTexture.hpp"

namespace RGL{


    RenderPassVk::RenderPassVk(const RenderPassConfig& config) : config(config), textures{config.attachments.size(), nullptr} {
       
    }

	RenderPassVk::~RenderPassVk()
	{
      
	}

    void RenderPassVk::SetAttachmentTexture(uint32_t index, ITexture* texture)
    {
        textures.at(index) = static_cast<TextureVk*>(texture);
    }

    void RenderPassVk::SetDepthAttachmentTexture(ITexture* texture)
    {
        depthTexture = static_cast<TextureVk*>(texture);
    }

    void RenderPassVk::SetStencilAttachmentTexture(ITexture* texture)
    {
        stencilTexture = static_cast<TextureVk*>(texture);
    }

}

#endif