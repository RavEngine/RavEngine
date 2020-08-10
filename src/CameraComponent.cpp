#include "CameraComponent.hpp"
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/OgreCompositorManager2.h>
#include <OgreColourValue.h>
#include <OgreWindow.h>
#include "OgreStatics.hpp"

using namespace Ogre;

CameraComponent::CameraComponent(float inFOV, float inNearClip, float inFarClip) : FOV(inFOV), nearClip(inNearClip), farClip(inFarClip), Component() {
	auto const id = to_string(uuids::uuid_system_generator{}());
	cam = GameplayStatics::ogreFactory.createCamera(id);
	cam->setFOVy(Ogre::Radian(FOV));
	cam->setNearClipDistance(nearClip);
	cam->setFarClipDistance(farClip);
}

void CameraComponent::setActive(bool newState) {
	active = newState;
	if (compositor != nullptr) {
		compositor->setEnabled(newState);
	}
}