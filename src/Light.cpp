#include "Light.hpp"
#include "Debug.hpp"
#include "PhysXDefines.h"

using namespace RavEngine;

void DirectionalLight::DebugDraw() const{
	auto pos = Ref<Entity>(getOwner())->transform();
	DebugDraw::DrawCapsule(pos->CalculateWorldMatrix(), 0x00FF00FF, 1, 2);
}

void AmbientLight::DebugDraw() const{
	auto pos = Ref<Entity>(getOwner())->transform();
	
	DebugDraw::DrawSphere(pos->CalculateWorldMatrix(), 0x00FF00FF, 1);
}
