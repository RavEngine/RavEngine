#if RGL_MTL_AVAILABLE
#import <Metal/Metal.h>
#include "MTLSampler.hpp"
#include "MTLDevice.hpp"
#include "RGLMTL.hpp"

namespace RGL{

MTLSamplerAddressMode rgl2mtladdressmode(RGL::SamplerAddressMode mode){
    switch(mode){
        case decltype(mode)::Wrap: return MTLSamplerAddressModeRepeat;
        case decltype(mode)::Mirror: return MTLSamplerAddressModeMirrorRepeat;
        case decltype(mode)::Clamp: return MTLSamplerAddressModeClampToEdge;
        case decltype(mode)::Border: return MTLSamplerAddressModeClampToBorderColor;
        case decltype(mode)::MirrorOnce: return MTLSamplerAddressModeMirrorClampToEdge;
    }
}

MTLSamplerBorderColor rgl2mtlsamplerbordercolor(const float color[4]){
    if (color[0] == 0 && color[1] == 0 && color[2] == 0){
        if (color[3] == 0){
            return MTLSamplerBorderColorTransparentBlack;
        }
        else{
            return MTLSamplerBorderColorOpaqueBlack;
        }
    }
    return MTLSamplerBorderColorOpaqueWhite;
}

SamplerMTL::SamplerMTL(decltype(owningDevice) owningDevice, const SamplerConfig& config) : owningDevice(owningDevice){
    MTLSamplerDescriptor *samplerDescriptor = [MTLSamplerDescriptor new];
    samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
    samplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
    samplerDescriptor.sAddressMode = rgl2mtladdressmode(config.addressModeU);
    samplerDescriptor.tAddressMode = rgl2mtladdressmode(config.addressModeV);
    samplerDescriptor.rAddressMode = rgl2mtladdressmode(config.addressModeW);
    
    samplerDescriptor.compareFunction = rgl2mtlcomparefunction(config.compareFunction);
    
    samplerDescriptor.borderColor = rgl2mtlsamplerbordercolor(config.borderColor);
     
    sampler = [owningDevice->device newSamplerStateWithDescriptor:samplerDescriptor];
}
}
#endif
