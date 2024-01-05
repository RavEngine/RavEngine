#include "Light.hpp"
#include "DebugDrawer.hpp"
#include "PhysXDefines.h"
#include "MeshAsset.hpp"
#include "Entity.hpp"
#include "Transform.hpp"
#include "App.hpp"
#if !RVE_SERVER
#include <RGL/Texture.hpp>
#endif

using namespace RavEngine;
using namespace std;

void DirectionalLight::DebugDraw(RavEngine::DebugDrawer& dbg, const Transform& tr) const{
	dbg.DrawCapsule(tr.GetWorldMatrix(), debug_color, 1, 2);
}

void AmbientLight::DebugDraw(RavEngine::DebugDrawer& dbg, const Transform& tr) const{
	dbg.DrawSphere(tr.GetWorldMatrix(), debug_color, 1);
}


void PointLight::DebugDraw(RavEngine::DebugDrawer& dbg, const Transform& tr) const{
	dbg.DrawSphere(tr.GetWorldMatrix(), debug_color, CalculateRadius() * 2);
}

void SpotLight::DebugDraw(RavEngine::DebugDrawer& dbg, const Transform& tr) const{
#ifndef NDEBUG
	//dbg.DrawWireframeMesh(tr.GetWorldMatrix(), LightManager::spotLightMesh);
#endif
}

void SpotLight::AddInstanceData(float* offset) const{
	auto intensity = GetIntensity();
	intensity = intensity * intensity;
	auto angle = std::clamp(coneAngle, 0.f, 90.f);
    
    auto& color = GetColorRGBA();

	//[0:11] filled with affine transform
	offset[12] = color.R;
	offset[13] = color.G;
	offset[14] = color.B;
	offset[15] = angle;
	offset[16] = intensity;
	offset[17] = penumbraAngle;
	
	//the radius and intensity are derived in the shader by extracting the scale information
}

RavEngine::ShadowLight::ShadowLight()
{
#if !RVE_SERVER
	constexpr static auto dim = 4096;
	shadowData.pyramid = {dim};

	auto device = GetApp()->GetDevice();

	shadowData.shadowMap = device->CreateTexture({
		.usage = {.Sampled = true, .DepthStencilAttachment = true },
		.aspect = {.HasDepth = true },
		.width = dim,
		.height = dim,
		.format = RGL::TextureFormat::D32SFloat,
		.debugName = "Shadow Texture"
	});
#endif
}
