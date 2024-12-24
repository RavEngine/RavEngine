#if !RVE_SERVER
#include <RGL/RGL.hpp>
#include <RGL/Types.hpp>
#include <RGL/Texture.hpp>

#include "ScreenEffect.hpp"
#include "App.hpp"
#include "RenderEngine.hpp"
#include "Common3D.hpp"
#include "Format.hpp"

namespace RavEngine{

ScreenEffectBase::ScreenEffectBase(const std::string_view name, const ScreenEffectConfig & config, const ScreenEffectInternalConfig& internalConfig) {
    auto device = GetApp()->GetDevice();
    auto defaultVSH = GetApp()->GetRenderEngine().GetDefaultPostEffectVSH();
        
    auto layout = device->CreatePipelineLayout({
        .bindings = config.bindings,
        .constants = {
            {
               sizeof(BasePushConstantUBO) + config.pushConstantSize, 0, RGL::StageVisibility(RGL::StageVisibility::Vertex | RGL::StageVisibility::Fragment)
            }
        },
    });
    
    RGL::RenderPipelineDescriptor rpd{
        .stages = {
            {
                .type = RGL::ShaderStageDesc::Type::Vertex,
                .shaderModule = defaultVSH,
            },
            {
                .type = RGL::ShaderStageDesc::Type::Fragment,
                .shaderModule = LoadShaderByFilename(Format("{}_fsh",name), device),
            }
        },
        .vertexConfig = {
            .vertexBindings = {
                        {
                            .binding = 0,
                            .stride = sizeof(Vertex2D),
                        },
            },
            .attributeDescs = {
                {
                    .location = 0,
                    .binding = 0,
                    .offset = offsetof(Vertex2D,position),
                    .format = RGL::VertexAttributeFormat::R32G32_SignedFloat,
                },
            }
        },
        .inputAssembly = {
            .topology = RGL::PrimitiveTopology::TriangleList,
        },
        .rasterizerConfig = {
            .cullMode = RGL::CullMode::Back,
            .windingOrder = RGL::WindingOrder::Counterclockwise,
        },
        .colorBlendConfig = {
            .attachments = {
                {
                    .format = internalConfig.outputFormat,
                    .sourceColorBlendFactor = config.sourceColorBlendFactor,
                    .destinationColorBlendFactor = config.destinationColorBlendFactor,
                    .blendEnabled = true
                },
            }
        },  // no depth
        .pipelineLayout = layout,
    };

    pipeline = device->CreateRenderPipeline(rpd);
}

}
#endif
