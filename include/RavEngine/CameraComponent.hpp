#include "Component.hpp"
#include "Entity.hpp"
#include "GameplayStatics.hpp"
#include <stduuid/uuid.h>
class Camera;

class CameraComponent : public Component {
public:
	CameraComponent(float inFOV = 60, float inNearClip = 0.1, float inFarClip = 100);

	virtual ~CameraComponent() {}

	void AddHook(const WeakRef<Entity>& e) override{}

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


protected:
	bool active = false;
    Camera* filamentCam;

	//camera details
	float FOV;
	float nearClip;
	float farClip;
};
