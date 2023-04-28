#pragma once
#include <RGL/RenderPass.hpp>
#include "MTLObjCCompatLayer.hpp"

namespace RGL{
    struct ITexture;
    struct RenderPassMTL : public IRenderPass{
        APPLE_API_PTR(MTLRenderPassDescriptor) renderPassDescriptor = nullptr;
        RenderPassMTL(const RenderPassConfig& config);
        virtual ~RenderPassMTL(){}
        void SetAttachmentTexture(uint32_t index, ITexture* texture) final;
        
        void SetDepthAttachmentTexture(ITexture* texture) final;
        void SetStencilAttachmentTexture(ITexture* texture) final;
    };

}
