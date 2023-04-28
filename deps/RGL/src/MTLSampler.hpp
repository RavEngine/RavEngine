#pragma once
#include <RGL/Types.hpp>
#include <RGL/Sampler.hpp>
#include "MTLObjCCompatLayer.hpp"
#include <memory>

namespace RGL{
struct DeviceMTL;

struct SamplerMTL : public ISampler{
    OBJC_ID(MTLSamplerState) sampler = nullptr;
    
    const std::shared_ptr<DeviceMTL> owningDevice;
    SamplerMTL(decltype(owningDevice), const SamplerConfig&);
};
}
