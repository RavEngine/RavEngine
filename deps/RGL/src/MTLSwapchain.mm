#if RGL_MTL_AVAILABLE
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include "MTLSwapchain.hpp"

namespace RGL{

void SwapchainMTL::Resize(uint32_t width, uint32_t height){
    surface->layer.drawableSize = CGSizeMake(width, height);
    
}
void SwapchainMTL::GetNextImage(uint32_t* index) {
    auto next = [surface->layer nextDrawable];
    
    activeTextures[idx] = TextureMTL(next, {static_cast<uint32_t>([[next texture] width]), static_cast<uint32_t>([[next texture] height])});
    activeTextures[idx].texture = [activeTextures[idx].drawable texture];
    
    *index = this->idx;
    idx = (idx + 1) % activeTextures.size();
    
}

ITexture* SwapchainMTL::ImageAtIndex(uint32_t index) {
    return &activeTextures[index];
}

void SwapchainMTL::Present(const SwapchainPresentConfig& config) {
    [activeTextures[config.imageIndex].drawable present];
}

}

#endif
