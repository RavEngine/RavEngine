#include "CameraComponent.hpp"
#include <filament/Camera.h>
#include "RenderEngine.hpp"
#include <filament/Engine.h>
#include <utils/EntityManager.h>

using namespace RavEngine;
using namespace filament;
using namespace utils;

CameraComponent::CameraComponent(float inFOV, float inNearClip, float inFarClip) : FOV(inFOV), nearClip(inNearClip), farClip(inFarClip), Component() {
	
}

void RavEngine::CameraComponent::AddHook(const WeakRef<RavEngine::Entity>& e)
{
	filamentCam = RenderEngine::getEngine()->createCamera(e.get()->transform()->getEntity());
}

void CameraComponent::setActive(bool newState) {
	active = newState;
}

void RavEngine::CameraComponent::SetTargetSize(unsigned int inwidth, unsigned int inheight)
{
	width = inwidth;
	height = inheight;
	const float aspect = (float)width / height;
	filamentCam->setProjection(Camera::Projection::ORTHO, -aspect * zoom, aspect * zoom, -zoom, zoom, 0, 1);
}
