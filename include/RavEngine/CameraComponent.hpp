#include "Component.hpp"
#include "Entity.hpp"
#include "GameplayStatics.hpp"
#include <stduuid/uuid.h>

class CameraComponent : public Component {
protected:
	Ogre::Camera* cam = nullptr;
	Ogre::CompositorWorkspace* compositor = nullptr;
public:
	CameraComponent(float inFOV = 60, float inNearClip = 0.1, float inFarClip = 100);

	virtual ~CameraComponent() {
		cam->detachFromParent();
		OGRE_DELETE cam;
	}

	/**
	Get the backend camera directly. For internal use only. The engine will not be aware of changes made directly to this pointer.
	@return a pointer to the camera
	*/
	Ogre::Camera* const getCamera() const {
		return cam;
	}

	void AddHook(const WeakRef<Entity>& e) override{
		//add the camera to the root
		e.get()->transform()->getNode()->attachObject(cam);
	}

	void RegisterAllAlternateTypes() override{}

	/**
	Enable / disable this camera
	@param newState the new enabled state for this camera. The renderer will choose the first active camera as the camera to use when drawing.
	*/
	void setActive(bool newState);

	/**
	@returns if this camera is active
	*/
	bool isActive() {
		return active;
	}

	/**
	For internal use
	@return the compositor workspace for this camera.
	*/
	Ogre::CompositorWorkspace* const GetCompositor() {
		return compositor;
	}

protected:
	bool active = false;

	//camera details
	float FOV;
	float nearClip;
	float farClip;
};