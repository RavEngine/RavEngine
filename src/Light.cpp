#include "Light.hpp"
#include "DebugDraw.hpp"
#include "PhysXDefines.h"
#include <bgfx/bgfx.h>
#include "MeshAsset.hpp"

using namespace RavEngine;
using namespace std;

static constexpr color_t debug_color = 0x00FF00FF;

Ref<MeshAsset> LightManager::pointLightMesh;
Ref<MeshAsset> LightManager::spotLightMesh;
Ref<LightManager::PointLightShaderInstance> LightManager::pointLightShader;
Ref<LightManager::AmbientLightShaderInstance> LightManager::ambientLightShader;
Ref<LightManager::DirectionalLightShaderInstance> LightManager::directionalLightShader;
Ref<LightManager::SpotLightShaderInstance> LightManager::spotLightShader;
bgfx::VertexBufferHandle LightManager::screenSpaceQuadVert = BGFX_INVALID_HANDLE;
bgfx::IndexBufferHandle LightManager::screenSpaceQuadInd = BGFX_INVALID_HANDLE;

void LightManager::Init(){
	pointLightMesh = make_shared<MeshAsset>("sphere.obj");
	spotLightMesh = make_shared<MeshAsset>("lightcone.obj");
	pointLightShader = make_shared<PointLightShaderInstance>(Material::Manager::AccessMaterialOfType<PointLightShader>());
	ambientLightShader = make_shared<AmbientLightShaderInstance>(Material::Manager::AccessMaterialOfType<AmbientLightShader>());
	directionalLightShader = make_shared<DirectionalLightShaderInstance>(Material::Manager::AccessMaterialOfType<DirectionalLightShader>());
	spotLightShader = make_shared<SpotLightShaderInstance>(Material::Manager::AccessMaterialOfType<SpotLightShader>());
	
	constexpr uint16_t indices[] = {0,2,1, 2,3,1};
	constexpr Vertex vertices[] = {{-1,-1,0}, {-1,1,0}, {1,-1,0}, {1,1,0}};
	bgfx::VertexLayout vl;
	vl.begin()
	.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
	.end();
	
	screenSpaceQuadVert = bgfx::createVertexBuffer(bgfx::copy(vertices, sizeof(vertices)), vl);
	screenSpaceQuadInd = bgfx::createIndexBuffer(bgfx::copy(indices, sizeof(indices)));
}

void DirectionalLight::DebugDraw(RavEngine::DebugDraw& dbg) const{
	auto pos = Ref<Entity>(getOwner())->transform();
	dbg.DrawCapsule(pos->CalculateWorldMatrix(), debug_color, 1, 2);
}

void DirectionalLight::AddInstanceData(float* offset) const{
	auto rot = Ref<Entity>(getOwner())->transform()->Up();
	
	offset[0] = color.R;
	offset[1] = color.G;
	offset[2] = color.B;
	offset[3] = Intensity;
	offset[4] = rot.x;
	offset[5] = rot.y;
	offset[6] = rot.z;
}

void AmbientLight::DebugDraw(RavEngine::DebugDraw& dbg) const{
	auto pos = Ref<Entity>(getOwner())->transform();
	
	dbg.DrawSphere(pos->CalculateWorldMatrix(), debug_color, 1);
}

void AmbientLight::AddInstanceData(float* offset) const{
	offset[0] = color.R;
	offset[1] = color.G;
	offset[2] = color.B;
	offset[3] = Intensity;
}

void PointLight::DebugDraw(RavEngine::DebugDraw& dbg) const{
	auto pos = Ref<Entity>(getOwner())->transform();
	dbg.DrawSphere(pos->CalculateWorldMatrix(), debug_color, CalculateRadius() * 2);
}

void PointLight::AddInstanceData(float* offset) const{
	//scale = radius
	auto trns = getOwner().lock()->transform();
	auto radius = CalculateRadius();
	auto worldMat = glm::scale(trns->CalculateWorldMatrix(), vector3(radius,radius,radius));
			
	//set [0:15] with transform matrix
	copyMat4(glm::value_ptr(worldMat), offset);
		
	offset[16] = color.R;
	offset[17] = color.G;
	offset[18] = color.B;
	offset[19] = Intensity;
}

void SpotLight::DebugDraw(RavEngine::DebugDraw&) const{
	//auto pos = getOwner().lock()->transform();
}

void SpotLight::AddInstanceData(float* offset) const{
	auto trns = getOwner().lock()->transform();
	auto intensity = Intensity.load();
	intensity = intensity * intensity;
	auto r = radius.load();
	auto worldMat = glm::scale(trns->CalculateWorldMatrix(), vector3(r,intensity,r));
	
	auto ptr1 = glm::value_ptr(worldMat);
	
	//don't need to send the last value of each row, because it is always [0,0,0,1] and can be reconstructed in shader
	offset[0] = ptr1[0];
	offset[1] = ptr1[1];
	offset[2] = ptr1[2];
	
	offset[3] = ptr1[4];
	offset[4] = ptr1[5];
	offset[5] = ptr1[6];
	
	offset[6] = ptr1[8];
	offset[7] = ptr1[9];
	offset[8] = ptr1[10];
	
	offset[9] = ptr1[12];
	offset[10] = ptr1[13];
	offset[11] = ptr1[14];
		
	//set remaining data
	offset[12] = color.R;
	offset[13] = color.G;
	offset[14] = color.B;
	offset[15] = r;
	offset[16] = intensity;
	offset[17] = penumbra.load();
	
	//the radius and intensity are derived in the shader by extracting the scale information
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

void SpotLight::Draw(int view){
	LightManager::spotLightShader->Draw(LightManager::spotLightMesh->getVertexBuffer(), LightManager::spotLightMesh->getIndexBuffer(), matrix4(), view);
}
