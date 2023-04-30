#include "Skybox.hpp"
#include "MeshAsset.hpp"
#include "RenderEngine.hpp"
#include "GetApp.hpp"
#include <RGL/Pipeline.hpp>
#include <RGL/Device.hpp>
#include "App.hpp"

using namespace RavEngine;
using namespace std;

STATIC(Skybox::defaultSkyMesh);
constexpr static uint32_t width = 640, height = 480;

void Skybox::Init() {
	defaultSkyMesh = MeshAsset::Manager::Get("skydome.obj");
}

void Skybox::Teardown() {
	defaultSkyMesh.reset();
}

Skybox::Skybox() : skyMat(std::make_shared<DefaultSkyMaterialInstance>(Material::Manager::Get<DefaultSkyMaterial>())), skyMesh(defaultSkyMesh) {}

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
                    .format = RGL::TextureFormat::BGRA8_Unorm
                },
            }
        },
        .depthWriteEnabled = false,
        .depthCompareFunction = RGL::DepthCompareFunction::Greater
    }
    )
{
}
