#include "Light.hpp"
#include "DebugDrawer.hpp"
#include "PhysXDefines.h"
#include "MeshAsset.hpp"
#include "Entity.hpp"
#include "Transform.hpp"
#include "App.hpp"
#if !RVE_SERVER
#include <RGL/Texture.hpp>
#include "Texture.hpp"
#include "RenderEngine.hpp"
#endif

#if _WIN32
#include <Windows.h>
#endif

using namespace RavEngine;
using namespace std;

const glm::mat4 pointLightViewMats[] = {
                {// +x
                    glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 1.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f })
                },
                {// -x
                    glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ -1.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f })
                },
                {// +y
                    glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, -1.0f })
                },
                {// -y
                    glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, -1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f })
                },
            // +Z
            {
                glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f })
           },
            // -z
                {
                glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f })
            }
};
#if _WIN32
void DumpMats() {
    for (const auto& mat : pointLightViewMats) {
        OutputDebugStringA("mat4(");
        for (int r = 0; r < 4; r++) {
            OutputDebugStringA("vec4(");
            for (int c = 0; c < 4; c++) {
                OutputDebugStringA(std::to_string(mat[r][c]).data());
                if (c < 3) {
                    OutputDebugStringA(", ");
                }
            }
            OutputDebugStringA(")");
            if (r < 3) {
                OutputDebugStringA(", ");
            }
        }
        OutputDebugStringA("),\n");
    }
}
#endif

void DirectionalLight::DebugDraw(RavEngine::DebugDrawer& dbg, const Transform& tr) const{
	dbg.DrawCapsule(tr.GetWorldMatrix(), debug_color, 1, 2);
}

void AmbientLight::DebugDraw(RavEngine::DebugDrawer& dbg, const Transform& tr) const{
	dbg.DrawSphere(tr.GetWorldMatrix(), debug_color, 1);
}


void PointLight::DebugDraw(RavEngine::DebugDrawer& dbg, const Transform& tr) const{
    auto radius = CalculateRadius();
	dbg.DrawSphere(tr.GetWorldMatrix(), debug_color, radius);
}


void SpotLight::DebugDraw(RavEngine::DebugDrawer& dbg, const Transform& tr) const{
    dbg.DrawSphere(tr.GetWorldMatrix(), debug_color, 1);
    dbg.DrawArrow(tr.GetWorldPosition(), tr.GetWorldPosition() + -tr.WorldUp() * 2.f, debug_color);
#ifndef NDEBUG
	//dbg.DrawWireframeMesh(tr.GetWorldMatrix(), LightManager::spotLightMesh);
#endif
}


RavEngine::DirectionalLight::DirectionalLight()
{
#if !RVE_SERVER
	constexpr static auto dim = 4096;
    {
        int i = 0;
        for(auto& pyramid : shadowData.pyramid){
            pyramid = {dim,Format("Shadowmap Cascasde {} Depth Pyramid", i++)};
        }
    }

	auto device = GetApp()->GetDevice();

    {
        int i = 0;
        for(auto& shadowMap : shadowData.shadowMap){
            shadowMap = device->CreateTexture({
                .usage = {.Sampled = true, .DepthStencilAttachment = true },
                .aspect = {.HasDepth = true },
                .width = dim,
                .height = dim,
                .format = RGL::TextureFormat::D32SFloat,
                .debugName = Format("Shadow Cascade {} Texture", i++)
            });
        }
    }
#endif
}

matrix4 RavEngine::SpotLight::CalcProjectionMatrix() const{
    matrix4 ret(1);
#if !RVE_SERVER
    ret = RMath::perspectiveProjection<float>(coneAngle * 2, 1, 0.1, 100);
#endif
    return ret;
}

matrix4 RavEngine::PointLight::CalcProjectionMatrix() const {
#if !RVE_SERVER
    return RMath::perspectiveProjection<float>(deg_to_rad(90), 1, 0.1, 100);
#else
    return matrix4{ 1.0f };
#endif
}

matrix4 RavEngine::PointLight::CalcViewMatrix(const vector3& lightPos, uint8_t index) {
#if !RVE_SERVER
    // center around light
    return glm::translate(pointLightViewMats[index], -lightPos);
#else
    return matrix4{ 1.0f };
#endif

}


matrix4 RavEngine::SpotLight::CalcViewMatrix(const matrix4& worldTransform) const{
    matrix4 ret(1);
#if !RVE_SERVER
    // -y is forward for spot lights, so we need to rotate to compensate
    const auto rotmat = glm::toMat4(quaternion(vector3(-3.14159265358 / 2, 0, 0)));
    auto combinedMat = worldTransform * rotmat;

    ret = glm::inverse(combinedMat);
#endif
    return ret;
}

RavEngine::SpotLight::SpotLight()
{
#if !RVE_SERVER
    constexpr static auto dim = 4096;
    shadowData.pyramid = {dim,"Shadowmap Spot Light Depth Pyramid"};

    auto device = GetApp()->GetDevice();
    
    shadowData.shadowMap = device->CreateTexture({
        .usage = {.Sampled = true, .DepthStencilAttachment = true },
        .aspect = {.HasDepth = true },
        .width = dim,
        .height = dim,
        .format = RGL::TextureFormat::D32SFloat,
        .debugName = "Spot Light Shadow Texture"
    });
#endif
}

RavEngine::EnvironmentLight::EnvironmentLight(decltype(sky) sky, decltype(outputTexture) ot, decltype(irradianceTexture) it) : sky(sky), outputTexture(ot), irradianceTexture(it)
{
    if (auto app = GetApp()) {
#if !RVE_SERVER
        const auto envMips = outputTexture->GetRHITexturePointer()->GetNumMips();
        const auto irrMips = irradianceTexture->GetRHITexturePointer()->GetNumMips();
        Debug::Assert(envMips >= 4, "Environment maps must have at least 4 mip levels");
        Debug::Assert(irrMips == 1, "Irradiance map must have 1 mip");
        auto dim = ot->GetTextureSize();
        stagingTexture = New<Texture>(dim.width, dim.height,Texture::Config{ .mipLevels = envMips, .enableRenderTarget = true, .format = RGL::TextureFormat::RGBA16_Sfloat, .debugName = "env staging"});
        stagingDepthTexture = app->GetDevice()->CreateTexture({
            .usage = {.DepthStencilAttachment = true},
            .aspect = {.HasDepth = true},
            .width = dim.width,
            .height = dim.height,
            .mipLevels = envMips,
            .format = RGL::TextureFormat::D32SFloat,
            .debugName = "env staging depth"
         });
#endif
    }
}

RavEngine::EnvironmentLight::~EnvironmentLight()
{
#if !RVE_SERVER
    if (auto app = GetApp()) {
        app->GetRenderEngine().gcTextures.enqueue(stagingDepthTexture);
    }
#endif
}

RavEngine::PointLight::PointLight()
{
#if !RVE_SERVER

	constexpr static auto dim = 1024;
    {
        int i = 0;
        for (auto& facePyramid : shadowData.cubePyramids) {
            facePyramid = { dim, Format("Shadowmap Depth Pyramid Face {} Point Light", i++) };
        }
    }
	auto device = GetApp()->GetDevice();

    {
        int i = 0;
        for (auto& shadowMap : shadowData.cubeShadowmaps) {
            shadowMap = device->CreateTexture({
                .usage = {.TransferSource = true, .Sampled = true, .DepthStencilAttachment = true },
                .aspect = {.HasDepth = true },
                .width = dim,
                .height = dim,
                .format = RGL::TextureFormat::D32SFloat,
                .debugName = Format("Point Light Shadow Texture Face {}", i++)
            });
        }
    }

	shadowData.mapCube = device->CreateTexture({
		.usage = {.TransferDestination = true, .Sampled = true, .DepthStencilAttachment = true },
		.aspect = {.HasDepth = true },
		.width = dim,
		.height = dim,
		.arrayLayers = 6,
		.format = RGL::TextureFormat::D32SFloat,
		.isCubemap = true,
		.debugName = "Shadow Texture Cubemap Point Light"
	});
#endif
}


