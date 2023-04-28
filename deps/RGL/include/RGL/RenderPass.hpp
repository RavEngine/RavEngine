#pragma once
#include <RGL/Types.hpp>
#include "TextureFormat.hpp"
#include <vector>
#include <array>
#include <optional>

namespace RGL{
struct ITexture;


struct RenderPassConfig {
    struct AttachmentDesc {
        // formats, multisample count, load and store ops (see VkAttachmentDescription), and layout (from VkAttachmentReference)
        TextureFormat format;
        MSASampleCount sampleCount = MSASampleCount::C1;
        LoadAccessOperation loadOp = LoadAccessOperation::DontCare;
        StoreAccessOperation storeOp = StoreAccessOperation::DontCare;

        LoadAccessOperation stencilLoadOp = LoadAccessOperation::DontCare;
        StoreAccessOperation stencilStoreOp = StoreAccessOperation::DontCare;

        std::array<float, 4> clearColor{ 0,0,0, 1 };
    };
    std::vector<AttachmentDesc> attachments;

    std::optional<AttachmentDesc> depthAttachment, stencilAttachment;
};

struct IRenderPass {
    
    virtual void SetAttachmentTexture(uint32_t index, ITexture* texture) = 0;
    virtual void SetDepthAttachmentTexture(ITexture* texture) = 0;
    virtual void SetStencilAttachmentTexture(ITexture* texture) = 0;
};

RGLRenderPassPtr CreateRenderPass(const RenderPassConfig& config);

}
