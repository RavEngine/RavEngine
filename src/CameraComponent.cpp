#include "CameraComponent.hpp"
#include "RenderEngine.hpp"

using namespace RavEngine;

CameraComponent::CameraComponent(float inFOV, float inNearClip, float inFarClip) : FOV(inFOV), nearClip(inNearClip), farClip(inFarClip), Component() {
}

void CameraComponent::SetActive(bool newState) {
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
