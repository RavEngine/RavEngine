#include "Component.hpp"
#include "Entity.hpp"
#include "GameplayStatics.hpp"
#include "OgreStatics.hpp"
#include <stduuid/uuid.h>

class CameraComponent : public Component {
protected:
	Ogre::Camera* cam = nullptr;
public:
	CameraComponent(float inFOV = 60, float inNearClip = 0.1, float inFarClip = 100) : FOV(inFOV), nearClip(inNearClip), farClip(inFarClip), Component() {
		auto const id = to_string(uuids::uuid_system_generator{}());
		cam = GameplayStatics::ogreFactory.createCamera(id);
		cam->setFOVy(Ogre::Radian(FOV));
		cam->setNearClipDistance(nearClip);
		cam->setFarClipDistance(farClip);
	}

	virtual ~CameraComponent() {
		cam->detachFromParent();
		OGRE_DELETE cam;
	}

	void AddHook(const WeakRef<Entity>& e) override{
		//add the camera to the root
		e.get()->transform()->getNode()->attachObject(cam);
	}

	void RegisterAllAlternateTypes() override{}
	bool isActive = false;

	//camera details
	float FOV;
	float nearClip;
	float farClip;

	/**
	Using the camera's current state, set the view transform
	*/
	void SetViewTransform() {
		Ref<Entity> entity = owner.get();
		auto transform = entity->transform();
		auto pos = transform->GetWorldPosition();
		//calculate where to position the camera
		float view[16];
		transform->WorldMatrixToArray(view);
		/*auto rot = transform->GetWorldRotation();
		bx::mtxQuatTranslation(view, bx::Quaternion{(float)rot.x,(float)rot.y,(float)rot.z,(float)rot.w},bx::Vec3(pos.x,pos.y,pos.z));*/

		float proj[16];
		//bx::mtxProj(proj, FOV, GameplayStatics::width / GameplayStatics::height, nearClip, farClip, bgfx::getCaps()->homogeneousDepth);
		//bgfx::setViewTransform(0, view, proj);
	}
};