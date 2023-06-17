#include "BuiltinMaterials.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <RGL/Pipeline.hpp>
#include "RenderEngine.hpp"
#include "mathtypes.hpp"
#include "Texture.hpp"

using namespace RavEngine;

constexpr static uint32_t width = 640, height = 480;

RavEngine::PBRMaterial::PBRMaterial(const std::string_view vsh_name, const std::string_view fsh_name) : Material(vsh_name,fsh_name,MaterialConfig
    {
        .vertConfig = {
            .vertexBindings = {
                {
                    .binding = 0,
                    .stride = sizeof(VertexNormalUV),
                },
                {
                    .binding = 1,
                    .stride = sizeof(uint32_t),
                    .inputRate = RGL::InputRate::Instance,
                }
            },
            .attributeDescs = {
                {
                    .location = 0,
                    .binding = 0,
                    .offset = offsetof(VertexNormalUV,position),
                    .format = RGL::VertexAttributeFormat::R32G32B32_SignedFloat,
                },
                {
                    .location = 1,
                    .binding = 0,
                    .offset = offsetof(VertexNormalUV,normal),
                    .format = RGL::VertexAttributeFormat::R32G32B32_SignedFloat,
                },
                {
                    .location = 2,
                    .binding = 0,
                    .offset = offsetof(VertexNormalUV,uv),
                    .format = RGL::VertexAttributeFormat::R32G32_SignedFloat,
                },

                {
                    .location = 3,
                    .binding = 1,
                    .offset = 0,
                    .format = RGL::VertexAttributeFormat::R32_Uint,
                },
            }
        },
        .colorBlendConfig = {
            .attachments = {
                {
                    .format = RenderEngine::colorTexFormat
                },
                {
                    .format = RenderEngine::normalTexFormat
                },
            }
        },
        .bindings = {
            {
                .binding = 0,
                .type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::CombinedImageSampler,
                .stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
            },
             {
                .binding = 1,
                .type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::SampledImage,
                .stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
            },
            {
                .binding = 2,
                .type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::StorageBuffer,
                .stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Vertex
            }
        },
        .pushConstantSize = sizeof(PBRPushConstantData)
	}
	) 
{
}

RavEngine::PBRMaterialInstance::PBRMaterialInstance(Ref<PBRMaterial> m) : MaterialInstance(m) {
    textureBindings[0] = Texture::Manager::defaultTexture;
}
