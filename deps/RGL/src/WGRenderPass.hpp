#pragma once
#include <RGL/RenderPass.hpp>

namespace RGL{
    struct ITexture;
    struct RenderPassWG : public IRenderPass{
        RenderPassWG(const RenderPassConfig& config);
        virtual ~RenderPassWG(){}
        void SetAttachmentTexture(uint32_t index, ITexture* texture) final;
        
        void SetDepthAttachmentTexture(ITexture* texture) final;
        void SetStencilAttachmentTexture(ITexture* texture) final;
    };
}
