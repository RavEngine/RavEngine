#include "CameraComponent.hpp"
#include "RenderEngine.hpp"

using namespace RavEngine;
using namespace utils;

CameraComponent::CameraComponent(float inFOV, float inNearClip, float inFarClip) : FOV(inFOV), nearClip(inNearClip), farClip(inFarClip), Component() {
}

void RavEngine::CameraComponent::AddHook(const WeakRef<RavEngine::Entity>& e)
{
}

void CameraComponent::setActive(bool newState) {
	active = newState;
}

void RavEngine::CameraComponent::SetTargetSize(unsigned int inwidth, unsigned int inheight)
{
	width = inwidth;
	height = inheight;
	const float aspect = (float)width / height;

	switch (projection) {
	case Mode::Perspective:
		break;
	case Mode::Orthographic:
		break;
	}
}
