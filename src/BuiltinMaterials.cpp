#include "BuiltinMaterials.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <RGL/Pipeline.hpp>
#include "RenderEngine.hpp"
#include "mathtypes.hpp"
#include "Texture.hpp"
#include "Utilities.hpp"
#include <ravengine_shader_defs.h>

using namespace RavEngine;

constexpr static uint32_t width = 640, height = 480;

STATIC(RavEngine::defaultVertexConfig) {
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
                .offset = offsetof(VertexNormalUV,tangent),
                .format = RGL::VertexAttributeFormat::R32G32B32_SignedFloat,
            },
            {
                .location = 3,
                .binding = 0,
                .offset = offsetof(VertexNormalUV,bitangent),
                .format = RGL::VertexAttributeFormat::R32G32B32_SignedFloat,
            },
            {
                .location = 4,
                .binding = 0,
                .offset = offsetof(VertexNormalUV,uv),
                .format = RGL::VertexAttributeFormat::R32G32_SignedFloat,
            },
            {
                .location = ENTITY_INPUT_LOCATION,
                .binding = 1,
                .offset = 0,
                .format = RGL::VertexAttributeFormat::R32_Uint,
            },
    }
};

STATIC(RavEngine::defaultColorBlendConfig) {
    .attachments = {
                {
                    .format = RenderEngine::colorTexFormat
                },
                {
                    .format = RenderEngine::normalTexFormat
                },
                {
                    .format = RenderEngine::normalTexFormat
                },
    }
};

RavEngine::PBRMaterial::PBRMaterial(const std::string_view vsh_name, const std::string_view fsh_name, PBRMaterialOptions options) : Material(vsh_name,fsh_name,MaterialConfig
    {
        .vertConfig = defaultVertexConfig,
        .colorBlendConfig = defaultColorBlendConfig,
        .bindings = {
            {
                .binding = 0,
                .type = RGL::BindingType::Sampler,
                .stageFlags = RGL::BindingVisibility::Fragment,
            },
             {
                .binding = 1,
                .type = RGL::BindingType::SampledImage,
                .stageFlags = RGL::BindingVisibility::Fragment,
            },
             {
                .binding = 2,
                .type = RGL::BindingType::SampledImage,
                .stageFlags = RGL::BindingVisibility::Fragment,
            },
            {
                .binding = 3,
                .type = RGL::BindingType::SampledImage,
                .stageFlags = RGL::BindingVisibility::Fragment,
            },
            {
                .binding = 4,
                .type = RGL::BindingType::SampledImage,
                .stageFlags = RGL::BindingVisibility::Fragment,
            },
            {
                .binding = 5,
                .type = RGL::BindingType::SampledImage,
                .stageFlags = RGL::BindingVisibility::Fragment,
            },
            {
                .binding = 6,
                .type = RGL::BindingType::SampledImage,
                .stageFlags = RGL::BindingVisibility::Fragment,
            },
            {
                .binding = MODEL_MATRIX_BINDING,
                .type = RGL::BindingType::StorageBuffer,
                .stageFlags = RGL::BindingVisibility::Vertex
            }
        },
        .pushConstantSize = sizeof(PBRPushConstantData),
        .cullMode = options.cullMode
	}
	) 
{
}

RavEngine::PBRMaterialInstance::PBRMaterialInstance(Ref<PBRMaterial> m) : MaterialInstance(m) {
    textureBindings[1] = Texture::Manager::defaultTexture;
    textureBindings[2] = Texture::Manager::defaultNormalTexture;
    textureBindings[3] = Texture::Manager::defaultTexture;
    textureBindings[4] = Texture::Manager::defaultTexture;
    textureBindings[5] = Texture::Manager::defaultTexture;
    textureBindings[6] = Texture::Manager::defaultTexture;
}
