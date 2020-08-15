#include "CameraComponent.hpp"
#include <filament/Camera.h>
#include "RenderEngine.hpp"
#include <filament/Engine.h>
#include <utils/EntityManager.h>
#include <filament/TransformManager.h>

using namespace RavEngine;
using namespace filament;
using namespace utils;

CameraComponent::CameraComponent(float inFOV, float inNearClip, float inFarClip) : FOV(inFOV), nearClip(inNearClip), farClip(inFarClip), Component() {
	filamentCam = RenderEngine::getEngine()->createCamera(EntityManager::get().create());
}

void RavEngine::CameraComponent::AddHook(const WeakRef<RavEngine::Entity>& e)
{
	filamentParentToEntity(e, filamentCam->getEntity());
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
