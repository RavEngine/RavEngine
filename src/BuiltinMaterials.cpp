#include "BuiltinMaterials.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <RGL/Pipeline.hpp>
#include "RenderEngine.hpp"

using namespace RavEngine;

constexpr static uint32_t width = 640, height = 480;

RavEngine::PBRMaterial::PBRMaterial(const std::string& name) : Material(name,
	{
		.vertConfig = {
            .vertexBindings = {{
                .binding = 0,
                .stride = sizeof(VertexNormalUV),
            }},
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
                }
            }
        }
	}
	) 
{
}
