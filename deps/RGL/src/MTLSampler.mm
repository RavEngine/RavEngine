#if RGL_MTL_AVAILABLE
#import <Metal/Metal.h>
#include "MTLSampler.hpp"
#include "MTLDevice.hpp"

namespace RGL{
SamplerMTL::SamplerMTL(decltype(owningDevice) owningDevice, const SamplerConfig& config) : owningDevice(owningDevice){
    MTLSamplerDescriptor *samplerDescriptor = [MTLSamplerDescriptor new];
    samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
    samplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
    samplerDescriptor.sAddressMode = MTLSamplerAddressModeRepeat;
    samplerDescriptor.tAddressMode = MTLSamplerAddressModeRepeat;
     
    sampler = [owningDevice->device newSamplerStateWithDescriptor:samplerDescriptor];
}
}
#endif
