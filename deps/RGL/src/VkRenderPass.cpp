#if RGL_VK_AVAILABLE
#include "VkRenderPass.hpp"
#include "RGLVk.hpp"
#include "VkTexture.hpp"

namespace RGL{


    RenderPassVk::RenderPassVk(const RenderPassConfig& config) : config(config), textures{config.attachments.size()} {
       
    }

	RenderPassVk::~RenderPassVk()
	{
      
	}

    void RenderPassVk::SetAttachmentTexture(uint32_t index, const TextureView& texture)
    {
        textures.at(index) = texture;
    }

    void RenderPassVk::SetDepthAttachmentTexture(const TextureView& texture)
    {
        depthTexture = texture;
    }

    void RenderPassVk::SetStencilAttachmentTexture(const TextureView& texture)
    {
        stencilTexture = texture;
    }

}

#endif