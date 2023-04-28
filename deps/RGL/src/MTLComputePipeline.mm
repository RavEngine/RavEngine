#if RGL_MTL_AVAILABLE
#import <Metal/Metal.h>
#include "MTLComputePipeline.hpp"
#include "MTLDevice.hpp"
#include "MTLShaderLibrary.hpp"
#include "RGLMTL.hpp"

namespace RGL{
    ComputePipelineMTL::ComputePipelineMTL(decltype(owningDevice) owningDevice, const RGL::ComputePipelineDescriptor &desc) : owningDevice(owningDevice) {
        
        auto shadermodule = desc.stage.shaderModule;
        MTL_CHECK(pipelineState = [owningDevice->device newComputePipelineStateWithFunction:shadermodule->function error:&err]);
    }
    ComputePipelineMTL::~ComputePipelineMTL() {
    
    }

}

#endif
