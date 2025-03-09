#pragma once
#include <RGL/Types.hpp>
#include "MTLObjCCompatLayer.hpp"
#include <RGL/Pipeline.hpp>
#include <unordered_map>

namespace RGL{
    struct DeviceMTL;
    
    struct PipelineLayoutMTL : public IPipelineLayout{
                
        PipelineLayoutDescriptor settings;
        PipelineLayoutMTL(const decltype(settings)& settings);
        
        std::unordered_map<uint32_t, uint32_t> samplerBindingsMap;
    };

    struct RenderPipelineMTL : public IRenderPipeline{
        OBJC_ID(MTLRenderPipelineState) pipelineState;
        OBJC_ID(MTLDepthStencilState) depthStencilState = nullptr;
        const std::shared_ptr<DeviceMTL> owningDevice;
        RenderPipelineDescriptor settings;
        APPLE_API_TYPE(MTLTriangleFillMode) currentFillMode;
        RenderPipelineMTL(decltype(owningDevice), const RenderPipelineDescriptor&);
    };

}
