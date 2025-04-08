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
                    .binding = VTX_POSITION_BINDING,
                    .stride = sizeof(VertexPosition_t),
                },
                {
                    .binding = VTX_NORMAL_BINDING,
                    .stride = sizeof(VertexNormal_t),
                },
                {
                    .binding = VTX_TANGENT_BINDING,
                    .stride = sizeof(VertexTangent_t),
                },
                {
                    .binding = VTX_BITANGENT_BINDING,
                    .stride = sizeof(VertexBitangent_t),
                },
                {
                    .binding = VTX_UV0_BINDING,
                    .stride = sizeof(VertexUV_t),
                },
                {
                    .binding = VTX_LIGHTMAP_BINDING,
                    .stride = sizeof(VertexUV_t),
                },
    },
        .attributeDescs = {
            {
                .location = 0,
                .binding = VTX_POSITION_BINDING,
                .offset = 0,
                .format = RGL::VertexAttributeFormat::R32G32B32_SignedFloat,
            },
            {
                .location = 1,
                .binding = VTX_NORMAL_BINDING,
                .offset = 0,
                .format = RGL::VertexAttributeFormat::R32G32B32_SignedFloat,
            },
             {
                .location = 2,
                .binding = VTX_TANGENT_BINDING,
                .offset = 0,
                .format = RGL::VertexAttributeFormat::R32G32B32_SignedFloat,
            },
            {
                .location = 3,
                .binding = VTX_BITANGENT_BINDING,
                .offset = 0,
                .format = RGL::VertexAttributeFormat::R32G32B32_SignedFloat,
            },
            {
                .location = 4,
                .binding = VTX_UV0_BINDING,
                .offset = 0,
                .format = RGL::VertexAttributeFormat::R32G32_SignedFloat,
            },
            {
                .location = 5,
                .binding = VTX_LIGHTMAP_BINDING,
                .offset = 0,
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
    }
};

STATIC(RavEngine::defaultColorBlendConfigUnlitPrepass) {
    .attachments = {}
};

STATIC(RavEngine::defaultColorBlendConfigLitPrepass) {
    .attachments = {
        {
            .format = RenderEngine::viewSpaceNormalTextureFormat
        },
    }
};

STATIC(RavEngine::defaultTransparentColorBlendConfig) {
    .attachments = {
#if RVE_TBDR
        {
            .format = RenderTargetCollection::formats[0],
        },
        {
            .format = RenderTargetCollection::formats[1],
        },
        {
            .format = RenderTargetCollection::formats[2],
        },
        {
            .format = RenderTargetCollection::formats[3],
        },
        {
            .format = RenderTargetCollection::mlabDepthFormat,
        },
#endif
        // no attachments on non-tilers
    }
};

STATIC(RavEngine::defaultUnlitColorBlendConfig) {
    .attachments = {
        {
            .format = RenderEngine::colorTexFormat
        },
    }
};

STATIC(RavEngine::defaultTransparentUnlitColorBlendConfig) {
    .attachments = {
#if RVE_TBDR
        {
            .format = RenderTargetCollection::formats[0],
        },
        {
            .format = RenderTargetCollection::formats[1],
        },
        {
            .format = RenderTargetCollection::formats[2],
        },
        {
            .format = RenderTargetCollection::formats[3],
        },
        {
            .format = RenderTargetCollection::mlabDepthFormat,
        },
#endif
        // no attachments on non-tilers
    }
};


RavEngine::PBRMaterialInstance::PBRMaterialInstance(Ref<PBRMaterial> m, uint32_t priority) : MaterialInstance(m, priority) {
    textureBindings[1] = Texture::Manager::defaultTexture;
    textureBindings[2] = Texture::Manager::defaultNormalTexture;
    textureBindings[3] = Texture::Manager::defaultTexture;
    textureBindings[4] = Texture::Manager::defaultTexture;
    textureBindings[5] = Texture::Manager::defaultTexture;
    textureBindings[6] = Texture::Manager::defaultTexture;
    textureBindings[7] = Texture::Manager::zeroTexture;
    textureBindings[8] = Texture::Manager::zeroTexture;
}

RavEngine::PBRMaterial::PBRMaterial(MaterialRenderOptions options, const std::string_view vsh_name, const std::string_view fsh_name) : LitMaterial(vsh_name, fsh_name,{
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
        {
            .binding = 8,
            .type = RGL::BindingType::SampledImage,
            .stageFlags = RGL::BindingVisibility::Fragment,
        },
    },
    .pushConstantSize = sizeof(PBRPushConstantData)
    }, options) {
}

RavEngine::PBRMaterial::PBRMaterial(MaterialRenderOptions options) : PBRMaterial(options, "pbr", options.opacityMode == OpacityMode::Transparent ? "pbr_transparent" : "pbr") {}

MaterialRenderOptions augmentOptionsForBaked(MaterialRenderOptions opt) {
    opt.requiredAttributes.lightmapUV = true;
    return opt;
}

RavEngine::PBRMaterialBaked::PBRMaterialBaked(MaterialRenderOptions options) : PBRMaterial(augmentOptionsForBaked(options), "pbr_baked", options.opacityMode == OpacityMode::Transparent ? "pbr_baked_transparent" : "pbr_baked")
{
}

#endif

