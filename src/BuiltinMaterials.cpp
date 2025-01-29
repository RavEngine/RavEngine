#if !RVE_SERVER
#include "BuiltinMaterials.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <RGL/Pipeline.hpp>
#include "RenderEngine.hpp"
#include "mathtypes.hpp"
#include "Texture.hpp"
#include "Utilities.hpp"
#include <ravengine_shader_defs.h>

using namespace RavEngine;

STATIC(RavEngine::defaultVertexConfig) {
    .vertexBindings = {
                {
                    .binding = 0,
                    .stride = sizeof(VertexNormalUV),
                },
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
    }
};

STATIC(RavEngine::defaultColorBlendConfig) {
    .attachments = {
                {
                    .format = RenderEngine::colorTexFormat
                },
                {
                    .format = RenderEngine::radianceFormat
                },
                {
                    .format = RenderEngine::albedoFormat
                },
                 {
                    .format = RenderEngine::viewSpaceNormalTextureFormat
                },
    }
};

STATIC(RavEngine::defaultTransparentColorBlendConfig) {
    .attachments = {}
};

STATIC(RavEngine::defaultUnlitColorBlendConfig) {
    .attachments = {
        {
            .format = RenderEngine::colorTexFormat
        },
    }
};

STATIC(RavEngine::defaultTransparentUnlitColorBlendConfig) {
    .attachments = {}   // no attachments
};


RavEngine::PBRMaterialInstance::PBRMaterialInstance(Ref<PBRMaterial> m, uint32_t priority) : MaterialInstance(m, priority) {
    textureBindings[1] = Texture::Manager::defaultTexture;
    textureBindings[2] = Texture::Manager::defaultNormalTexture;
    textureBindings[3] = Texture::Manager::defaultTexture;
    textureBindings[4] = Texture::Manager::defaultTexture;
    textureBindings[5] = Texture::Manager::defaultTexture;
    textureBindings[6] = Texture::Manager::defaultTexture;
    textureBindings[7] = Texture::Manager::zeroTexture;
}


RavEngine::PBRMaterial::PBRMaterial(MaterialRenderOptions options) : LitMaterial("pbr", options.opacityMode == OpacityMode::Transparent ? "pbr_transparent" : "pbr", {
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
            .binding = 7,
            .type = RGL::BindingType::SampledImage,
            .stageFlags = RGL::BindingVisibility::Fragment,
        },
    },
    .pushConstantSize = sizeof(PBRPushConstantData) 
    }, options) {}


#endif


