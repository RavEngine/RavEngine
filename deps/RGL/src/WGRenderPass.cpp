#if RGL_WEBGPU_AVAILABLE
#include "WGRenderPass.hpp"
#include "WGTexture.hpp"
#include "RGLWG.hpp"

namespace RGL{

WGPULoadOp rgl2wgload(LoadAccessOperation op){
    switch(op){
        case LoadAccessOperation::Load:
            return WGPULoadOp_Load;
        case LoadAccessOperation::Clear:
            return WGPULoadOp_Clear;
        case LoadAccessOperation::DontCare:
        case LoadAccessOperation::NotAccessed:
            return WGPULoadOp_Undefined;
    }
}

WGPUStoreOp rgl2wgstore(StoreAccessOperation op){
    switch(op){
        case decltype(op)::Store:
            return WGPUStoreOp_Store;
        case StoreAccessOperation::None:
        case decltype(op)::DontCare:
            return  WGPUStoreOp_Undefined;
        default:
            FatalError("selected StoreOp is not implemented");
    }
}


RenderPassWG::RenderPassWG(const RenderPassConfig& config){

    colorAttachments.resize(config.attachments.size());
    renderPass.colorAttachmentCount = colorAttachments.size();
    renderPass.colorAttachments = colorAttachments.data();
    renderPass.nextInChain = &maxDrawCount.chain;
    {
        uint32_t i = 0;
        for(const auto& attachmentdesc : config.attachments){
            auto& attachment = colorAttachments.at(i);
            attachment.loadOp = rgl2wgload(attachmentdesc.loadOp);
            attachment.storeOp = rgl2wgstore(attachmentdesc.storeOp);
            attachment.clearValue = {attachmentdesc.clearColor[0],attachmentdesc.clearColor[1],attachmentdesc.clearColor[2],attachmentdesc.clearColor[3]};
            i++;
        }
    }
    //TODO: depthstencil
}

void RenderPassWG::SetAttachmentTexture(uint32_t index, const TextureView& texture){
    colorAttachments.at(index).view = texture.texture.wg;
}
        
void RenderPassWG::SetDepthAttachmentTexture(const TextureView& texture){
    depthStencilAttachment.view = texture.texture.wg;
}
void RenderPassWG::SetStencilAttachmentTexture(const TextureView& texture){
    SetDepthAttachmentTexture(texture);
}

}
#endif