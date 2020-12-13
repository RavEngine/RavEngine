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

void DirectionalLight::AddInstanceData(float* offset) const{
	auto tr = Ref<Entity>(getOwner())->transform();
	auto rot = glm::eulerAngles(tr->GetWorldRotation());
	
	offset[0] = color.R;
	offset[1] = color.G;
	offset[2] = color.B;
	offset[3] = Intensity;
	offset[4] = rot.x;
	offset[5] = rot.y;
	offset[6] = rot.z;
}

void AmbientLight::DebugDraw() const{
	auto pos = Ref<Entity>(getOwner())->transform();
	
	DebugDraw::DrawSphere(pos->CalculateWorldMatrix(), debug_color, 1);
}

void AmbientLight::AddInstanceData(float* offset) const{
	offset[0] = color.R;
	offset[1] = color.G;
	offset[2] = color.B;
	offset[3] = Intensity;
}

void PointLight::DebugDraw() const{
	auto pos = Ref<Entity>(getOwner())->transform();
	DebugDraw::DrawSphere(pos->CalculateWorldMatrix(), debug_color, CalculateRadius());
}

void PointLight::AddInstanceData(float* offset) const{
	//scale = radius
	auto pos = Ref<Entity>(getOwner())->transform();
	auto radius = CalculateRadius();
	auto worldMat = glm::scale(pos->CalculateWorldMatrix(), vector3(radius,radius,radius));
	
	//set [0:15] with transform matrix
	copyMat4(glm::value_ptr(worldMat), offset);
		
	offset[16] = color.R;
	offset[17] = color.G;
	offset[18] = color.B;
	offset[19] = Intensity;
}

void DirectionalLight::Draw(int view){
	LightManager::directionalLightShader->Draw(LightManager::screenSpaceQuadVert, LightManager::screenSpaceQuadInd, matrix4(), view);
}

void PointLight::Draw(int view){
	LightManager::pointLightShader->Draw(LightManager::pointLightMesh->getVertexBuffer(), LightManager::pointLightMesh->getIndexBuffer(), matrix4(),view);
}

void AmbientLight::Draw(int view){
	LightManager::ambientLightShader->Draw(LightManager::screenSpaceQuadVert, LightManager::screenSpaceQuadInd, matrix4(), view);
}
