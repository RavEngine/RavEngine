#if RGL_MTL_AVAILABLE
#import <Metal/Metal.h>
#include "MTLComputePipeline.hpp"
#include "MTLDevice.hpp"
#include "MTLShaderLibrary.hpp"
#include "RGLMTL.hpp"

namespace RGL{
    ComputePipelineMTL::ComputePipelineMTL(decltype(owningDevice) owningDevice, const RGL::ComputePipelineDescriptor &desc) : owningDevice(owningDevice), settings(desc) {
        
        auto shadermodule = desc.stage.shaderModule;
        auto cdesc = [MTLComputePipelineDescriptor new];
        cdesc.computeFunction = shadermodule->function;
        if (desc.debugName.data() != nullptr){
            [cdesc setLabel:[NSString stringWithUTF8String:desc.debugName.data()]];
        }
        
        MTL_CHECK(pipelineState = [owningDevice->device newComputePipelineStateWithDescriptor:cdesc options:MTLPipelineOptionNone reflection:nil error:&err]);
    }
    ComputePipelineMTL::~ComputePipelineMTL() {
    
    }

}

#endif
