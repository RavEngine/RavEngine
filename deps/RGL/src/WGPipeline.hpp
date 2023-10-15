#pragma once
#include <RGL/Types.hpp>
#include <RGL/Pipeline.hpp>
#include <emscripten/html5_webgpu.h>

namespace RGL{
    struct DeviceWG;
    
    struct PipelineLayoutWG : public IPipelineLayout{
                
        PipelineLayoutDescriptor settings;
        PipelineLayoutWG(const decltype(settings)& settings) : settings(settings){}        
    };

    struct RenderPipelineWG : public IRenderPipeline{
        const std::shared_ptr<DeviceWG> owningDevice;
        WGPURenderPipeline renderPipeline;
        RenderPipelineDescriptor settings;
        RenderPipelineWG(decltype(owningDevice), const RenderPipelineDescriptor&);

        ~RenderPipelineWG();
    };

}
