#if RGL_WEBGPU_AVAILABLE
#include "WGRenderPass.hpp"

namespace RGL{
RenderPassWG::RenderPassWG(const RenderPassConfig& config){

}

void RenderPassWG::SetAttachmentTexture(uint32_t index, ITexture* texture){

}
        
void RenderPassWG::SetDepthAttachmentTexture(ITexture* texture){
    
}
void RenderPassWG::SetStencilAttachmentTexture(ITexture* texture){
    
}

}
#endif