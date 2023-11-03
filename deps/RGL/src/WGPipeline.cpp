#if RGL_WEBGPU_AVAILABLE
#include "WGPipeline.hpp"
#include "WGDevice.hpp"
#include "WGShaderLibrary.hpp"
#include "RGLWG.hpp"

namespace RGL{

    WGPUVertexFormat rgl2wgvx(VertexAttributeFormat format){
        switch(format){
            case decltype(format)::Undefined:
                FatalError("'Undefined' verted format passed");
                break;
            case decltype(format)::R32_Uint:
                return WGPUVertexFormat_Uint32;
            case decltype(format)::R32G32_SignedFloat:
                return WGPUVertexFormat_Float32x2;
            case decltype(format)::R32G32B32_SignedFloat:
                return WGPUVertexFormat_Float32x3;
            case decltype(format)::R32G32B32A32_SignedFloat:
                return WGPUVertexFormat_Float32x4;
            default:
                FatalError("Format is not implemented");
        }
    }

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

    // binding to stepmode
    struct VertexBufferLayoutData{
        WGPUVertexBufferLayout layout;
        std::vector<WGPUVertexAttribute> attributes;
    };
    std::unordered_map<uint32_t, VertexBufferLayoutData> vertexLayouts;
    {
        for(const auto& binding : desc.vertexConfig.vertexBindings){
            auto& entry = vertexLayouts[binding.binding];
            entry.layout.stepMode = binding.inputRate == RGL::InputRate::Instance ? WGPUVertexStepMode_Instance : WGPUVertexStepMode_Vertex;
            entry.layout.arrayStride = binding.stride;
        }
    }

    for(const auto& attribute : desc.vertexConfig.attributeDescs){
        auto& entry = vertexLayouts.at(attribute.binding);
        WGPUVertexAttribute attr;
        attr.offset = attribute.offset;
        attr.shaderLocation = attribute.location;
        attr.format = rgl2wgvx(attribute.format);

        entry.attributes.push_back(attr);

    }
    // finalize attribute structure
    std::vector<WGPUVertexBufferLayout> vertexLayoutsLinear;
    for(auto& [binding, entry]: vertexLayouts){
        entry.layout.attributeCount = entry.attributes.size();
        entry.layout.attributes = entry.attributes.data();
        vertexLayoutsLinear.push_back(entry.layout);        // copy into linear for webgpu
    }
    


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
        .entryPoint = "main",
        .bufferCount = vertexLayoutsLinear.size(),
        .buffers = vertexLayoutsLinear.data()
    };

    pipelineDesc.multisample = {
        .count = 1              // TODO: support multisample
    };

    renderPipeline = wgpuDeviceCreateRenderPipeline(owningDevice->device, &pipelineDesc);
}
RenderPipelineWG::~RenderPipelineWG(){
    wgpuRenderPipelineRelease(renderPipeline);
}

}

#endif