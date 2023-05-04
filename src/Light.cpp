#include "Light.hpp"
#include "DebugDrawer.hpp"
#include "PhysXDefines.h"
#include "MeshAsset.hpp"
#include "Entity.hpp"
#include "Transform.hpp"

using namespace RavEngine;
using namespace std;

void DirectionalLight::DebugDraw(RavEngine::DebugDrawer& dbg, const Transform& tr) const{
	dbg.DrawCapsule(tr.CalculateWorldMatrix(), debug_color, 1, 2);
}

void AmbientLight::DebugDraw(RavEngine::DebugDrawer& dbg, const Transform& tr) const{
	dbg.DrawSphere(tr.CalculateWorldMatrix(), debug_color, 1);
}


void PointLight::DebugDraw(RavEngine::DebugDrawer& dbg, const Transform& tr) const{
	dbg.DrawSphere(tr.CalculateWorldMatrix(), debug_color, CalculateRadius() * 2);
}

void SpotLight::DebugDraw(RavEngine::DebugDrawer& dbg, const Transform& tr) const{
#ifndef NDEBUG
	//dbg.DrawWireframeMesh(tr.CalculateWorldMatrix(), LightManager::spotLightMesh);
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