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


RavEngine::UnidirectionalShadowLight::UnidirectionalShadowLight()
{
#if !RVE_SERVER
	constexpr static auto dim = 4096;
	shadowData.pyramid = {dim,"Shadowmap Depth Pyramid"};

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

RavEngine::PointLight::PointLight()
{
#if !RVE_SERVER

	constexpr static auto dim = 4096;
	for (auto& facePyramid : shadowData.cubePyramids) {
		facePyramid = { dim, "Shadowmap Depth Pyramid Face Point Light" };
	}
	auto device = GetApp()->GetDevice();

	for (auto& shadowMap : shadowData.cubeShadowmaps) {
		shadowMap = device->CreateTexture({
			.usage = {.TransferSource = true, .Sampled = true, .DepthStencilAttachment = true },
			.aspect = {.HasDepth = true },
			.width = dim,
			.height = dim,
			.format = RGL::TextureFormat::D32SFloat,
			.debugName = "Shadow Texture"
		});
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