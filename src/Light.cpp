#include "Light.hpp"
#include "Debug.hpp"
#include "PhysXDefines.h"
#include <bgfx/bgfx.h>
#include "MeshAsset.hpp"

using namespace RavEngine;

static constexpr color_t debug_color = 0x00FF00FF;

Ref<MeshAsset> LightManager::pointLightMesh;
Ref<LightManager::PointLightShaderInstance> LightManager::pointLightShader;
Ref<LightManager::AmbientLightShaderInstance> LightManager::ambientLightShader;
Ref<LightManager::DirectionalLightShaderInstance> LightManager::directionalLightShader;
bgfx::VertexBufferHandle LightManager::screenSpaceQuadVert = BGFX_INVALID_HANDLE;
bgfx::IndexBufferHandle LightManager::screenSpaceQuadInd = BGFX_INVALID_HANDLE;

void LightManager::Init(){
	pointLightMesh = new MeshAsset("sphere.obj");
	pointLightShader = new PointLightShaderInstance(Material::Manager::AccessMaterialOfType<PointLightShader>());
	ambientLightShader = new AmbientLightShaderInstance(Material::Manager::AccessMaterialOfType<AmbientLightShader>());
	directionalLightShader = new DirectionalLightShaderInstance(Material::Manager::AccessMaterialOfType<DirectionalLightShader>());
	
	const uint16_t indices[] = {0,2,1, 2,3,1};
	const Vertex vertices[] = {{-1,-1,0}, {-1,1,0}, {1,-1,0}, {1,1,0}};
	bgfx::VertexLayout vl;
	vl.begin()
	.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
	.end();
	
	screenSpaceQuadVert = bgfx::createVertexBuffer(bgfx::copy(vertices, sizeof(vertices)), vl);
	screenSpaceQuadInd = bgfx::createIndexBuffer(bgfx::copy(indices, sizeof(indices)));
}

void DirectionalLight::DebugDraw() const{
	auto pos = Ref<Entity>(getOwner())->transform();
	DebugDraw::DrawCapsule(pos->CalculateWorldMatrix(), debug_color, 1, 2);
}

void DirectionalLight::DrawVolume(int view) const{
	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_CULL_CW |
				   BGFX_STATE_BLEND_ADD);
	
	auto tr = Ref<Entity>(getOwner())->transform();
	auto rot = glm::eulerAngles(tr->GetWorldRotation());
	
	LightManager::directionalLightShader->SetColorDirection({color.R,color.B,color.G,Intensity}, {static_cast<float>(rot.x),static_cast<float>(rot.y),static_cast<float>(rot.z),0});
	LightManager::directionalLightShader->Draw(LightManager::screenSpaceQuadVert, LightManager::screenSpaceQuadInd, matrix4(), view);
}

void AmbientLight::DebugDraw() const{
	auto pos = Ref<Entity>(getOwner())->transform();
	
	DebugDraw::DrawSphere(pos->CalculateWorldMatrix(), debug_color, 1);
}

void AmbientLight::DrawVolume(int view) const{
	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_CULL_CW |
				   BGFX_STATE_BLEND_ADD);
	LightManager::ambientLightShader->SetColor({color.R,color.B,color.G,Intensity});
	LightManager::ambientLightShader->Draw(LightManager::screenSpaceQuadVert, LightManager::screenSpaceQuadInd, matrix4(), view);
}

void PointLight::DebugDraw() const{
	auto pos = Ref<Entity>(getOwner())->transform();
	DebugDraw::DrawSphere(pos->CalculateWorldMatrix(), debug_color, CalculateRadius());
}

void PointLight::DrawVolume(int view) const{
	//scale = radius
	auto pos = Ref<Entity>(getOwner())->transform();
	auto radius = CalculateRadius();
	auto worldMat = glm::scale(pos->CalculateWorldMatrix(), vector3(radius,radius,radius));
	
	auto center = pos->GetWorldPosition();
	
	LightManager::pointLightShader->SetPosColor({static_cast<float>(center.x),static_cast<float>(center.y),static_cast<float>(center.z),radius}, {color.R,color.B,color.G,Intensity});
	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_DEPTH_TEST_GEQUAL | BGFX_STATE_CULL_CCW |
				   BGFX_STATE_BLEND_ADD);
	LightManager::pointLightShader->Draw(LightManager::pointLightMesh->getVertexBuffer(), LightManager::pointLightMesh->getIndexBuffer(), worldMat,view);
}
