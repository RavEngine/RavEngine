#if !RVE_SERVER
#include <RGL/RGL.hpp>
#include <RGL/Types.hpp>
#include <RGL/Texture.hpp>

#include "PostProcess.hpp"
#include "App.hpp"
#include "RenderEngine.hpp"
#include "Common3D.hpp"

namespace RavEngine{

PostProcessPassInstance::~PostProcessPassInstance() = default;
PostProcessPassInstance::PostProcessPassInstance(Ref<PostProcessPass> effect, const PostProcessInstanceConfig& config) : effect(effect), inputConfiguration(config.inputconfiguration), outputConfiguration(config.outputConfiguration), clearOutputBeforeRendering(config.clearOutputBeforeRendering){}

PostProcessPass::PostProcessPass(const std::string_view name, const PostProcessConfig& config){
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
                .shaderModule = LoadShaderByFilename(StrFormat("{}.fsh",name), device),
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
                    .format = RenderEngine::colorTexFormat,
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