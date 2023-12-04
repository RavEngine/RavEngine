#if !RVE_SERVER

#include "Skybox.hpp"
#include "MeshAsset.hpp"
#include "RenderEngine.hpp"
#include "GetApp.hpp"
#include <RGL/Pipeline.hpp>
#include <RGL/Device.hpp>
#include "App.hpp"

using namespace RavEngine;
using namespace std;


Skybox::Skybox() : skyMat(std::make_shared<DefaultSkyMaterialInstance>(Material::Manager::Get<DefaultSkyMaterial>())) {}

RavEngine::ISkyMaterial::ISkyMaterial(const std::string& shaderpath) : Material(shaderpath, 
    {
        .vertConfig = {
            .vertexBindings = {{
                .binding = 0,
                .stride = sizeof(Vertex),
            }},
            .attributeDescs = {
                {
                    .location = 0,
                    .binding = 0,
                    .offset = offsetof(Vertex,position),
                    .format = RGL::VertexAttributeFormat::R32G32B32_SignedFloat,
                },
            }
        },
        .colorBlendConfig = {
            .attachments = {
                {
                    .format = RenderEngine::colorTexFormat
                },
            }
        },
        .depthWriteEnabled = false,
        .depthCompareFunction = RGL::DepthCompareFunction::Equal,
        .verbatimConfig = true
    }
    )
{
}

#endif
