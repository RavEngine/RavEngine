#include "Light.hpp"
#include "Debug.hpp"
#include "PhysXDefines.h"

using namespace RavEngine;

static constexpr color_t debug_color = 0x00FF00FF;

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
	//TODO: draw light volume
	//scale = radius
}
