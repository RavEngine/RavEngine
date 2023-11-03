#pragma once
#include <RGL/RenderPass.hpp>
#include "MTLObjCCompatLayer.hpp"

namespace RGL{
    struct ITexture;
    struct RenderPassMTL : public IRenderPass{
        APPLE_API_PTR(MTLRenderPassDescriptor) renderPassDescriptor = nullptr;
        RenderPassMTL(const RenderPassConfig& config);
        virtual ~RenderPassMTL(){}
        void SetAttachmentTexture(uint32_t index, const TextureView& texture) final;
        
        void SetDepthAttachmentTexture(const TextureView& texture) final;
        void SetStencilAttachmentTexture(const TextureView& texture) final;
    };

}
