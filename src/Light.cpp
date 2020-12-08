#include "Light.hpp"
#include "Debug.hpp"
#include "PhysXDefines.h"
#include <bgfx/bgfx.h>
#include "MeshAsset.hpp"

using namespace RavEngine;

static constexpr color_t debug_color = 0x00FF00FF;

Ref<MeshAsset> LightManager::pointLightMesh;
Ref<LightManager::PointLightShaderInstance> LightManager::pointLightShader;

void LightManager::Init(){
	pointLightMesh = new MeshAsset("sphere.obj");
	pointLightShader = new PointLightShaderInstance(Material::Manager::AccessMaterialOfType<PointLightShader>());
}

void DirectionalLight::DebugDraw() const{
	auto pos = Ref<Entity>(getOwner())->transform();
	DebugDraw::DrawCapsule(pos->CalculateWorldMatrix(), debug_color, 1, 2);
}

void DirectionalLight::DrawVolume(int view) const{
	
}

void AmbientLight::DebugDraw() const{
	auto pos = Ref<Entity>(getOwner())->transform();
	
	DebugDraw::DrawSphere(pos->CalculateWorldMatrix(), debug_color, 1);
}

void AmbientLight::DrawVolume(int view) const{
	//uniform shape over entire view
}

void PointLight::DebugDraw() const{
	auto pos = Ref<Entity>(getOwner())->transform();
	DebugDraw::DrawSphere(pos->CalculateWorldMatrix(), debug_color, radius);
}

void PointLight::DrawVolume(int view) const{
	//scale = radius
	auto pos = Ref<Entity>(getOwner())->transform();
	auto worldMat = glm::scale(pos->CalculateWorldMatrix(), vector3(radius,radius,radius));
	
	LightManager::pointLightShader->Draw(LightManager::pointLightMesh->getVertexBuffer(), LightManager::pointLightMesh->getIndexBuffer(), worldMat,view);
}
