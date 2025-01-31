#if !RVE_SERVER
#include "Material.hpp"
#include <sstream>
#include <fstream>
#include <RenderEngine.hpp>
#include "mathtypes.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Filesystem.hpp"
#include "Common3D.hpp"
#include "App.hpp"
#include <typeindex>
#include "Debug.hpp"
#include "VirtualFileSystem.hpp"
#include <RGL/RGL.hpp>
#include <RGL/Device.hpp>
#include <RGL/Pipeline.hpp>
#include <ravengine_shader_defs.h>
#include "CaseAnalysis.hpp"

using namespace std;
namespace RavEngine {

    /**
    Create a material given a shader. Also registers it in the material manager
    @param shader the path to the shader
    */
    Material::Material(const std::string_view name, const MaterialConfig& config) : Material(name, name, config) {}

    RavEngine::Material::Material(const std::string_view vsh_name, const std::string_view fsh_name, const MaterialConfig& config) : opacityMode(config.opacityMode)
    {
        auto device = GetApp()->GetDevice();

        //get all shader files for this programs
        auto vertshaderName = Format("{}_vsh", vsh_name);
        auto fragShaderName = Format("{}_fsh", fsh_name);

        auto vertShader = LoadShaderByFilename(vertshaderName, device);
        auto fragShader = LoadShaderByFilename(fragShaderName, device);

        auto samplerPtr = GetApp()->GetRenderEngine().textureSampler;

        auto configBindingsCopy = config.bindings;

        if (!config.verbatimConfig) {
            // must have the model matrix binding
            configBindingsCopy.push_back(
                {
                    .binding = MODEL_MATRIX_BINDING,
                    .type = RGL::BindingType::StorageBuffer,
                    .stageFlags = RGL::BindingVisibility::VertexFragment
                }
            );
        }

        RGL::PipelineLayoutDescriptor pld{
            .bindings = configBindingsCopy,
        };

        if (config.pushConstantSize > 0) {
            pld.constants.push_back({
                 size_t(config.pushConstantSize), 0, RGL::StageVisibility(RGL::StageVisibility::Vertex | RGL::StageVisibility::Fragment)
            });
        }

        pipelineLayout = device->CreatePipelineLayout(pld);

        bool hasDepthPrepass = config.opacityMode == OpacityMode::Opaque && !config.verbatimConfig;

        auto vertexConfigCopy = config.vertConfig;
        if (!config.verbatimConfig) {
            // must have the entity ID buffer
            vertexConfigCopy.attributeDescs.push_back({
                .location = ENTITY_INPUT_LOCATION,
                .binding = ENTITY_INPUT_BINDING,
                .offset = 0,
                .format = RGL::VertexAttributeFormat::R32_Uint,
                });
            vertexConfigCopy.vertexBindings.push_back({
                .binding = ENTITY_INPUT_BINDING,
                .stride = sizeof(uint32_t),
                .inputRate = RGL::InputRate::Instance,
                });
        }

        RGL::RenderPipelineDescriptor rpd{
            .stages = {
                {
                    .type = RGL::ShaderStageDesc::Type::Vertex,
                    .shaderModule = vertShader,
                },
                {
                    .type = RGL::ShaderStageDesc::Type::Fragment,
                    .shaderModule = fragShader,
                }
            },
            .vertexConfig = vertexConfigCopy,
            .inputAssembly = {
                .topology = RGL::PrimitiveTopology::TriangleList,
            },
            .rasterizerConfig = {
                .cullMode = config.cullMode,
                .windingOrder = RGL::WindingOrder::Counterclockwise,
            },
            .colorBlendConfig = config.colorBlendConfig,
            .depthStencilConfig = {
                .depthFormat = RGL::TextureFormat::D32SFloat,
                .depthTestEnabled = config.depthTestEnabled,
                .depthWriteEnabled = IsTransparent() ? false : config.depthWriteEnabled,
                .depthFunction = hasDepthPrepass? RGL::DepthCompareFunction::Equal : config.depthCompareFunction,
            },
            .pipelineLayout = pipelineLayout,
            .debugName = Format("Material {}, {}",vsh_name, fsh_name)
        };

        renderPipeline = device->CreateRenderPipeline(rpd);

        // invert some settings for the shadow pipeline
        if (hasDepthPrepass) {
            const auto sh_name = Format("{}_fsh_depthonly", fsh_name);
            rpd.stages[1].shaderModule = LoadShaderByFilename(sh_name, device);  // use the shadow variant
        }
        else {
            rpd.stages.pop_back();  // no fragment shader
        }

        rpd.colorBlendConfig.attachments.clear();                           // only write to depth
        rpd.depthStencilConfig.depthFunction = config.depthCompareFunction; // use real depth compare function

        rpd.debugName = Format("Material {} {} (Depth Prepass)", fsh_name, vsh_name);
        depthPrepassPipeline = device->CreateRenderPipeline(rpd);

        rpd.rasterizerConfig.windingOrder = RGL::WindingOrder::Clockwise;   // backface shadows
        rpd.rasterizerConfig.depthClampEnable = true;                       // clamp out-of-view fragments
        rpd.debugName = Format("Material {} {} (Shadow)", fsh_name, vsh_name);
        shadowRenderPipeline = device->CreateRenderPipeline(rpd);
    }

    Material::~Material() {
        // enqueue the pipeline and layout for deletion
        if (auto app = GetApp()) {
            auto& renderer = app->GetRenderEngine();
            renderer.gcPipelineLayout.enqueue(pipelineLayout);
            renderer.gcRenderPipeline.enqueue(renderPipeline);
            renderer.gcRenderPipeline.enqueue(shadowRenderPipeline);
        }
    }

std::vector<RGL::PipelineLayoutDescriptor::LayoutBindingDesc> augmentLitMaterialBindings(const std::vector<RGL::PipelineLayoutDescriptor::LayoutBindingDesc>& bindings, OpacityMode opacityMode) {
        auto configBindingsCopy = bindings;
        configBindingsCopy.push_back(
            {
                .binding = 11,
                .type = RGL::BindingType::StorageBuffer,
                .stageFlags = RGL::BindingVisibility::VertexFragment
            }
        );
        configBindingsCopy.push_back(
            {
                .binding = 12,
                .type = RGL::BindingType::StorageBuffer,
                .stageFlags = RGL::BindingVisibility::Fragment
            }
        );
        configBindingsCopy.push_back(
            {
                .binding = 13,
                .type = RGL::BindingType::StorageBuffer,
                .stageFlags = RGL::BindingVisibility::Fragment
            }
        );
        configBindingsCopy.push_back(
            {
                .binding = 14,
                .type = RGL::BindingType::Sampler,
                .stageFlags = RGL::BindingVisibility::Fragment
            }
        );
        configBindingsCopy.push_back(
            {
                .binding = 15,
                .type = RGL::BindingType::StorageBuffer,
                .stageFlags = RGL::BindingVisibility::Fragment
            }
        );
        configBindingsCopy.push_back(
            {
                .binding = 16,
                .type = RGL::BindingType::StorageBuffer,
                .stageFlags = RGL::BindingVisibility::Fragment
            }
        );
        configBindingsCopy.push_back(
            {
                .binding = 17,
                .type = RGL::BindingType::StorageBuffer,
                .stageFlags = RGL::BindingVisibility::Fragment
            }
        );
        if (opacityMode == OpacityMode::Transparent){ // storage images for MLAB
            configBindingsCopy.push_back(
                                         {
                                             .binding = 23,
                                             .type = RGL::BindingType::StorageImage,
                                             .stageFlags = RGL::BindingVisibility::Fragment
                                         }
                                         );
            configBindingsCopy.push_back(
                                         {
                                             .binding = 24,
                                             .type = RGL::BindingType::StorageImage,
                                             .stageFlags = RGL::BindingVisibility::Fragment
                                         }
                                         );
            configBindingsCopy.push_back(
                                         {
                                             .binding = 25,
                                             .type = RGL::BindingType::StorageImage,
                                             .stageFlags = RGL::BindingVisibility::Fragment
                                         }
                                         );
            configBindingsCopy.push_back(
                                         {
                                             .binding = 26,
                                             .type = RGL::BindingType::StorageImage,
                                             .stageFlags = RGL::BindingVisibility::Fragment
                                         }
                                         );
            configBindingsCopy.push_back(
                                         {
                                             .binding = 27,
                                             .type = RGL::BindingType::StorageImage,
                                             .stageFlags = RGL::BindingVisibility::Fragment
                                         }
                                         );
        }
        configBindingsCopy.push_back(
            {
                .binding = 28,
                .type = RGL::BindingType::StorageBuffer,
                .stageFlags = RGL::BindingVisibility::Fragment
            }
        );
        configBindingsCopy.push_back(
            {
                .binding = 29,
                .type = RGL::BindingType::StorageBuffer,
                .stageFlags = RGL::BindingVisibility::Fragment
            }
        );
        configBindingsCopy.push_back(
            {
                .binding = 30,
                .type = RGL::BindingType::StorageBuffer,
                .stageFlags = RGL::BindingVisibility::Fragment
            }
        );
        configBindingsCopy.push_back(
            {
                .binding = 1,
                .count = 2048,
                .isBindless = true,     // binding 0 set 1
                .type = RGL::BindingType::SampledImage,
                .stageFlags = RGL::BindingVisibility::Fragment
            }
        );
        configBindingsCopy.push_back(
            {
                .binding = 2,
                .count = 2048,
                .isBindless = true,     // binding 0 set 1
                .type = RGL::BindingType::SampledImage,
                .stageFlags = RGL::BindingVisibility::Fragment
            }
        );
        return configBindingsCopy;
    }

    RavEngine::LitMaterial::LitMaterial(const std::string_view vsh_name, const std::string_view fsh_name, const PipelineOptions& pipeOptions, const MaterialRenderOptions& options) : Material(vsh_name, fsh_name, MaterialConfig
        {
            .vertConfig = defaultVertexConfig,
            .colorBlendConfig = options.opacityMode == OpacityMode::Opaque ? defaultColorBlendConfig : defaultTransparentColorBlendConfig,
            .bindings = augmentLitMaterialBindings(pipeOptions.bindings, options.opacityMode),
            .pushConstantSize = pipeOptions.pushConstantSize,
            .cullMode = options.cullMode,
            .opacityMode = options.opacityMode
        }
    )
    {
    }

    std::vector<RGL::PipelineLayoutDescriptor::LayoutBindingDesc> augmentUnlitMaterialBindings(const std::vector<RGL::PipelineLayoutDescriptor::LayoutBindingDesc>& bindings, OpacityMode opacityMode) {
        auto configBindingsCopy = bindings;
        configBindingsCopy.push_back(
            {
                .binding = 11,
                .type = RGL::BindingType::StorageBuffer,
                .stageFlags = RGL::BindingVisibility::VertexFragment
            }
        );
        
        if (opacityMode == OpacityMode::Transparent){ // storage images for MLAB
            configBindingsCopy.push_back(
                                         {
                                             .binding = 23,
                                             .type = RGL::BindingType::StorageImage,
                                             .stageFlags = RGL::BindingVisibility::Fragment
                                         }
                                         );
            configBindingsCopy.push_back(
                                         {
                                             .binding = 24,
                                             .type = RGL::BindingType::StorageImage,
                                             .stageFlags = RGL::BindingVisibility::Fragment
                                         }
                                         );
            configBindingsCopy.push_back(
                                         {
                                             .binding = 25,
                                             .type = RGL::BindingType::StorageImage,
                                             .stageFlags = RGL::BindingVisibility::Fragment
                                         }
                                         );
            configBindingsCopy.push_back(
                                         {
                                             .binding = 26,
                                             .type = RGL::BindingType::StorageImage,
                                             .stageFlags = RGL::BindingVisibility::Fragment
                                         }
                                         );
            configBindingsCopy.push_back(
                                         {
                                             .binding = 27,
                                             .type = RGL::BindingType::StorageImage,
                                             .stageFlags = RGL::BindingVisibility::Fragment
                                         }
                                         );
        }
        return configBindingsCopy;
    }

    UnlitMaterial::UnlitMaterial(const std::string_view vsh_name, const std::string_view fsh_name, const PipelineOptions& pipeOptions, const MaterialRenderOptions& options)
        : Material(vsh_name, fsh_name, MaterialConfig{
            .vertConfig = defaultVertexConfig,
            .colorBlendConfig = options.opacityMode == OpacityMode::Opaque ? defaultUnlitColorBlendConfig : defaultTransparentUnlitColorBlendConfig,
            .bindings = augmentUnlitMaterialBindings(pipeOptions.bindings, options.opacityMode),
            .pushConstantSize = pipeOptions.pushConstantSize,
            .cullMode = options.cullMode,
            .opacityMode = options.opacityMode
            }) {}


    PipelineUseConfiguration RavEngine::MaterialVariant::GetShadowRenderPipeline() const
    {
        RGLRenderPipelinePtr pipeline;

        std::visit(CaseAnalysis{
            [&pipeline](const Ref<LitMaterial>& m) {
                pipeline = m->GetShadowRenderPipeline();
            },
            [&pipeline](const Ref<UnlitMaterial>& m) {
                pipeline = m->GetShadowRenderPipeline();
            } }, variant);

        return { pipeline };
    }

    PipelineUseConfiguration RavEngine::MaterialVariant::GetMainRenderPipeline() const
    {
        RGLRenderPipelinePtr pipeline;

        std::visit(CaseAnalysis{
            [&pipeline](const Ref<LitMaterial>& m) {
                pipeline = m->GetMainRenderPipeline();
            },
            [&pipeline](const Ref<UnlitMaterial>& m) {
                pipeline = m->GetMainRenderPipeline();
            } }, variant);

        return { pipeline };
    }

    PipelineUseConfiguration RavEngine::MaterialVariant::GetDepthPrepassPipeline() const
    {
        RGLRenderPipelinePtr pipeline;

        std::visit(CaseAnalysis{
            [&pipeline](const Ref<LitMaterial>& m) {
                pipeline = m->GetDepthPrepassPipeline();
            },
            [&pipeline](const Ref<UnlitMaterial>& m) {
                pipeline = m->GetDepthPrepassPipeline();
            } }, variant);

        return { pipeline };
    }
}
#endif

