#include "CameraComponent.hpp"


CameraComponent::CameraComponent(float inFOV, float inNearClip, float inFarClip) : FOV(inFOV), nearClip(inNearClip), farClip(inFarClip), Component() {
}

void CameraComponent::setActive(bool newState) {
	active = newState;
}