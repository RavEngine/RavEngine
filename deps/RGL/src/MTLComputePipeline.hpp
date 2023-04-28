#pragma once
#include <RGL/Pipeline.hpp>
#include "MTLObjCCompatLayer.hpp"
#include <RGL/Types.hpp>

namespace RGL{
struct DeviceMTL;
struct ComputePipelineMTL : public IComputePipeline{
    OBJC_ID(MTLComputePipelineState) pipelineState;
    const std::shared_ptr<DeviceMTL> owningDevice;
    ComputePipelineMTL(decltype(owningDevice) owningDevice, const ComputePipelineDescriptor& desc);
    virtual ~ComputePipelineMTL();
};
}
