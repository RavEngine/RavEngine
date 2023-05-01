#include "BuiltinMaterials.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <RGL/Pipeline.hpp>
#include "RenderEngine.hpp"
#include "mathtypes.hpp"

using namespace RavEngine;

constexpr static uint32_t width = 640, height = 480;

RavEngine::PBRMaterial::PBRMaterial(const std::string& name) : Material(name,
    {
        .vertConfig = {
            .vertexBindings = {
                {
                    .binding = 0,
                    .stride = sizeof(VertexNormalUV),
                },
                {
                    .binding = 1,
                    .stride = sizeof(glm::mat4)
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
                    .format = RGL::VertexAttributeFormat::R32G32B32A32_SignedFloat,
                },
                {
                    .location = 4,
                    .binding = 1,
                    .offset = sizeof(glm::vec4),
                    .format = RGL::VertexAttributeFormat::R32G32B32A32_SignedFloat,
                },
                {
                    .location = 5,
                    .binding = 1,
                    .offset = sizeof(glm::vec4) * 2,
                    .format = RGL::VertexAttributeFormat::R32G32B32A32_SignedFloat,
                },
                {
                    .location = 6,
                    .binding = 1,
                    .offset = sizeof(glm::vec4) * 3,
                    .format = RGL::VertexAttributeFormat::R32G32B32A32_SignedFloat,
                }
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
        }
	}
	) 
{
}
