#if RGL_MTL_AVAILABLE
#import <Metal/Metal.h>
#include "MTLPipeline.hpp"
#include "MTLDevice.hpp"
#include "RGLMTL.hpp"
#include "MTLShaderLibrary.hpp"
#include <simd/simd.h>

namespace RGL{

std::pair<MTLVertexFormat,uint32_t>  rgl2mtlvx(VertexAttributeFormat format){
    switch(format){
        case decltype(format)::Undefined:
            FatalError("'Undefined' verted format passed");
            break;
        case decltype(format)::R32_Uint:
            return std::make_pair(MTLVertexFormatUInt, sizeof(uint32_t));
            break;
        case decltype(format)::R32G32_SignedFloat:
            return std::make_pair(MTLVertexFormatFloat2, sizeof(float)*2);
        case decltype(format)::R32G32B32_SignedFloat:
            return std::make_pair(MTLVertexFormatFloat3, sizeof(float)*3);
    }
}

MTLBlendOperation rgl2mtlblend(decltype(RenderPipelineDescriptor::ColorBlendConfig::ColorAttachmentConfig::colorBlendOperation) config){
    switch(config){
        case decltype(config)::Add:             return MTLBlendOperationAdd;
        case decltype(config)::Subtract:        return MTLBlendOperationSubtract;
        case decltype(config)::ReverseSubtract: return MTLBlendOperationReverseSubtract;
        case decltype(config)::Min:             return MTLBlendOperationMin;
        case decltype(config)::Max:             return MTLBlendOperationMax;
        default:
            FatalError("Invalid blend operation");
    }
}

MTLBlendFactor rgl2mtlfactor(decltype(RenderPipelineDescriptor::ColorBlendConfig::ColorAttachmentConfig::sourceColorBlendFactor) config){
    switch(config){
        case decltype(config)::Zero:                    return MTLBlendFactorZero;
        case decltype(config)::One:                     return MTLBlendFactorOne;
        case decltype(config)::SourceColor:             return MTLBlendFactorSourceColor;
        case decltype(config)::OneMinusSourceColor:     return MTLBlendFactorOneMinusSourceColor;
        case decltype(config)::DestColor:               return MTLBlendFactorDestinationColor;
        case decltype(config)::OneMinusDestColor:       return MTLBlendFactorOneMinusDestinationColor;
        case decltype(config)::SourceAlpha:             return MTLBlendFactorSourceAlpha;
        case decltype(config)::OneMinusSourceAlpha:     return MTLBlendFactorOneMinusSourceAlpha;
        case decltype(config)::DestAlpha:               return MTLBlendFactorDestinationAlpha;
        case decltype(config)::OneMinusDestAlpha:       return MTLBlendFactorOneMinusDestinationAlpha;
        case decltype(config)::ConstantColor:           return MTLBlendFactorBlendColor;
        case decltype(config)::OneMinusConstantColor:   return MTLBlendFactorOneMinusBlendColor;
        case decltype(config)::ConstantAlpha:           return MTLBlendFactorBlendAlpha;
        case decltype(config)::OneMinusConstantAlpha:   return MTLBlendFactorOneMinusBlendAlpha;
        case decltype(config)::SourceAlphaSaturate:     return MTLBlendFactorSourceAlphaSaturated;
        case decltype(config)::Source1Color:            return MTLBlendFactorSource1Color;
        case decltype(config)::OneMinusSource1Color:    return MTLBlendFactorOneMinusSource1Color;
        case decltype(config)::Source1Alpha:            return MTLBlendFactorSource1Alpha;
        case decltype(config)::OneMinusSource1Alpha:    return MTLBlendFactorOneMinusSource1Alpha;
    }
}

MTLCompareFunction rgl2mtlcomparefunction(DepthCompareFunction fn){
    switch(fn){
        case decltype(fn)::Never:           return MTLCompareFunctionNever;
        case decltype(fn)::Less:            return MTLCompareFunctionLess;
        case decltype(fn)::Equal:           return MTLCompareFunctionEqual;
        case decltype(fn)::LessOrEqual:     return MTLCompareFunctionLessEqual;
        case decltype(fn)::Greater:         return MTLCompareFunctionGreater;
        case decltype(fn)::NotEqual:        return MTLCompareFunctionNotEqual;
        case decltype(fn)::GreaterOrEqual:  return MTLCompareFunctionGreaterEqual;
        case decltype(fn)::Always:          return MTLCompareFunctionAlways;
    }
}

MTLTriangleFillMode rgl2MTLTriangleFillMode(RGL::PolygonOverride mode){
    switch(mode){
        case PolygonOverride::Fill:
            return MTLTriangleFillModeFill;
        case PolygonOverride::Line:
            return MTLTriangleFillModeLines;
        default:
            FatalError("Unsupported fill mode");
    }
}

RenderPipelineMTL::RenderPipelineMTL(decltype(owningDevice) owningDevice, const RenderPipelineDescriptor& desc) : owningDevice(owningDevice), settings(desc){
    auto pipelineDesc = [MTLRenderPipelineDescriptor new];
    
    currentFillMode = rgl2MTLTriangleFillMode(desc.rasterizerConfig.polygonOverride);
    
    id<MTLFunction> vertFunc = nullptr, fragFunc = nullptr;
    for(const auto stage : desc.stages){
        auto fn = std::static_pointer_cast<ShaderLibraryMTL>(stage.shaderModule)->function;
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
    
    [pipelineDesc setVertexFunction:vertFunc];
    [pipelineDesc setFragmentFunction:fragFunc];
    
    // create a single interleaved buffer descriptor
    {
        uint32_t i = 0;
        uint32_t totalStride = 0;
        auto vertexDescriptor = [MTLVertexDescriptor new];
        std::unordered_map<uint32_t, MTLVertexStepFunction> bindingSteps;
        {
            uint32_t i = 0;
            for(const auto& binding : desc.vertexConfig.vertexBindings){
                bindingSteps[binding.binding] = binding.inputRate == RGL::InputRate::Instance ? MTLVertexStepFunctionPerInstance : MTLVertexStepFunctionPerVertex;
                if (binding.inputRate == RGL::InputRate::Instance){
                    vertexDescriptor.layouts[i].stepFunction = MTLVertexStepFunctionPerInstance;
                    vertexDescriptor.layouts[i].stepRate = 1;
                    vertexDescriptor.layouts[i].stride = binding.stride;
                }
                i++;
            }
        }
        
        for(const auto& attribute : desc.vertexConfig.attributeDescs){
            auto vertexAttribute = [MTLVertexAttributeDescriptor new];
            auto formatpair = rgl2mtlvx(attribute.format);
            [vertexAttribute setFormat:formatpair.first];
            [vertexAttribute setOffset:attribute.offset];
            [vertexAttribute setBufferIndex: attribute.binding];
            
            if (bindingSteps.at(attribute.binding) == MTLVertexStepFunctionPerVertex){
                totalStride += formatpair.second;
            }
            
            vertexDescriptor.attributes[i] = vertexAttribute;
            i++;
        }
        
        vertexDescriptor.layouts[0].stride = totalStride;
        [pipelineDesc setVertexDescriptor:vertexDescriptor];
    }

    for(int i = 0; i < desc.colorBlendConfig.attachments.size(); i++){
        auto& attachment = desc.colorBlendConfig.attachments[i];
        auto attachmentDesc = pipelineDesc.colorAttachments[i];
        [attachmentDesc setPixelFormat:rgl2mtlformat(attachment.format)];
        
        [attachmentDesc setBlendingEnabled:attachment.blendEnabled];
        [attachmentDesc setRgbBlendOperation:rgl2mtlblend(attachment.colorBlendOperation)];
        [attachmentDesc setAlphaBlendOperation:rgl2mtlblend(attachment.alphaBlendOperation)];
        
        [attachmentDesc setSourceRGBBlendFactor:rgl2mtlfactor(attachment.sourceColorBlendFactor)];
        [attachmentDesc setSourceAlphaBlendFactor:rgl2mtlfactor(attachment.sourceAlphaBlendFactor)];
        [attachmentDesc setDestinationRGBBlendFactor:rgl2mtlfactor(attachment.destinationColorBlendFactor)];
        [attachmentDesc setDestinationAlphaBlendFactor:rgl2mtlfactor(attachment.destinationAlphaBlendFactor)];
    }
    
    pipelineDesc.depthAttachmentPixelFormat = rgl2mtlformat(desc.depthStencilConfig.depthFormat);
    pipelineDesc.stencilAttachmentPixelFormat = rgl2mtlformat(desc.depthStencilConfig.stencilFormat);
    
    MTL_CHECK(pipelineState = [owningDevice->device newRenderPipelineStateWithDescriptor:pipelineDesc error:&err]);
    
    if (desc.depthStencilConfig.depthTestEnabled){
        auto depthDesc = [MTLDepthStencilDescriptor new];
        depthDesc.depthCompareFunction = rgl2mtlcomparefunction(desc.depthStencilConfig.depthFunction);
        depthDesc.depthWriteEnabled = desc.depthStencilConfig.depthWriteEnabled;
        depthStencilState = [owningDevice->device newDepthStencilStateWithDescriptor:depthDesc];
    }
    
}

}
#endif
