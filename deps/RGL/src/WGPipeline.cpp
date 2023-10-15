#if RGL_WEBGPU_AVAILABLE
#include "WGPipeline.hpp"
#include "WGDevice.hpp"
#include "WGShaderLibrary.hpp"
#include "RGLWG.hpp"

namespace RGL{

RenderPipelineWG::RenderPipelineWG(decltype(owningDevice) owningDevice, const RenderPipelineDescriptor& desc) : owningDevice(owningDevice){
    WGPURenderPipelineDescriptor pipelineDesc{};
    pipelineDesc.nextInChain = nullptr;

    WGPUShaderModule vertFunc = nullptr, fragFunc = nullptr;
    for(const auto stage : desc.stages){
        auto fn = std::static_pointer_cast<ShaderLibraryWG>(stage.shaderModule)->shaderModule;
        switch(stage.type){
            case decltype(stage.type)::Vertex:
                vertFunc = fn;
                break;
            case decltype(stage.type)::Fragment:
                fragFunc = fn;
                break;
            default:
                FatalError("Stage type is not supported");
        }
    }

    //TODO: vertex bindings

    std::array<WGPUColorTargetState,16> colorTargetState;

    const auto nTargets = desc.colorBlendConfig.attachments.size();
    for(int i = 0; i < nTargets; i++){
        auto& attachment = desc.colorBlendConfig.attachments[i];

        auto& cts = colorTargetState.at(i);
        cts.format = WGPUTextureFormat_BGRA8Unorm; //TODO

        /*
        [attachmentDesc setBlendingEnabled:attachment.blendEnabled];
        [attachmentDesc setRgbBlendOperation:rgl2mtlblend(attachment.colorBlendOperation)];
        [attachmentDesc setAlphaBlendOperation:rgl2mtlblend(attachment.alphaBlendOperation)];
        
        [attachmentDesc setSourceRGBBlendFactor:rgl2mtlfactor(attachment.sourceColorBlendFactor)];
        [attachmentDesc setSourceAlphaBlendFactor:rgl2mtlfactor(attachment.sourceAlphaBlendFactor)];
        [attachmentDesc setDestinationRGBBlendFactor:rgl2mtlfactor(attachment.destinationColorBlendFactor)];
        [attachmentDesc setDestinationAlphaBlendFactor:rgl2mtlfactor(attachment.destinationAlphaBlendFactor)];
        */
    }
    
    WGPUFragmentState fragmentState{
        .module = fragFunc,
        .entryPoint = "main",
        .targetCount = nTargets,
        .targets = colorTargetState.data()
    };

    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.vertex = {
        .module = vertFunc, 
        .entryPoint = "main"
    };


    renderPipeline = wgpuDeviceCreateRenderPipeline(owningDevice->device, &pipelineDesc);
}
RenderPipelineWG::~RenderPipelineWG(){
    wgpuRenderPipelineRelease(renderPipeline);
}

}

#endif