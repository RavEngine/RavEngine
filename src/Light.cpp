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
	pointLightShader = make_shared<PointLightShaderInstance>(Material::Manager::GetMaterial<PointLightShader>());
	ambientLightShader = make_shared<AmbientLightShaderInstance>(Material::Manager::GetMaterial<AmbientLightShader>());
	directionalLightShader = make_shared<DirectionalLightShaderInstance>(Material::Manager::GetMaterial<DirectionalLightShader>());
	spotLightShader = make_shared<SpotLightShaderInstance>(Material::Manager::GetMaterial<SpotLightShader>());
	
	constexpr uint16_t indices[] = {0,2,1, 2,3,1};
	constexpr Vertex vertices[] = {{-1,-1,0}, {-1,1,0}, {1,-1,0}, {1,1,0}};
	bgfx::VertexLayout vl;
	vl.begin()
	.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
	.end();
	
	screenSpaceQuadVert = bgfx::createVertexBuffer(bgfx::copy(vertices, sizeof(vertices)), vl);
	screenSpaceQuadInd = bgfx::createIndexBuffer(bgfx::copy(indices, sizeof(indices)));
}

void LightManager::Teardown() {
	pointLightMesh.reset();
	spotLightMesh.reset();
	pointLightShader.reset();
	ambientLightShader.reset();
	directionalLightShader.reset();
	spotLightShader.reset();
	bgfx::destroy(screenSpaceQuadVert);
	bgfx::destroy(screenSpaceQuadInd);
}

void DirectionalLight::DebugDraw(RavEngine::DebugDraw& dbg) const{
	auto pos = Ref<Entity>(GetOwner())->GetTransform();
	dbg.DrawCapsule(pos->CalculateWorldMatrix(), debug_color, 1, 2);
}

void DirectionalLight::AddInstanceData(float* offset) const{
	offset[0] = color.R;
	offset[1] = color.G;
	offset[2] = color.B;
	offset[3] = Intensity;
}

void AmbientLight::DebugDraw(RavEngine::DebugDraw& dbg) const{
	auto pos = Ref<Entity>(GetOwner())->GetTransform();
	
	dbg.DrawSphere(pos->CalculateWorldMatrix(), debug_color, 1);
}

void AmbientLight::AddInstanceData(float* offset) const{
	offset[0] = color.R;
	offset[1] = color.G;
	offset[2] = color.B;
	offset[3] = Intensity;
}

void PointLight::DebugDraw(RavEngine::DebugDraw& dbg) const{
	auto pos = Ref<Entity>(GetOwner())->GetTransform();
	dbg.DrawSphere(pos->CalculateWorldMatrix(), debug_color, CalculateRadius() * 2);
}

void PointLight::AddInstanceData(float* offset) const{

	//[0:11] filled with affine transform
	offset[12] = color.R;
	offset[13] = color.G;
	offset[14] = color.B;
	offset[15] = Intensity;
}

void SpotLight::DebugDraw(RavEngine::DebugDraw&) const{
	//auto pos = getOwner().lock()->transform();
}

void SpotLight::AddInstanceData(float* offset) const{
	auto intensity = Intensity;
	intensity = intensity * intensity;
	auto r = radius;
	
		
	//[0:11] filled with affine transform
	offset[12] = color.R;
	offset[13] = color.G;
	offset[14] = color.B;
	offset[15] = r;
	offset[16] = intensity;
	offset[17] = penumbra;
	
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
