#include "RavEngine/Entity.hpp"
#include "RavEngine/CameraComponent.hpp"
#include "RavEngine/GameplayStatics.hpp"
#include "RavEngine/IInputListener.hpp"
#include "RavEngine/ScriptComponent.hpp"

class PlayerActor;


class PlayerScript : public RavEngine::ScriptComponent, public RavEngine::IInputListener {
public:
	Ref<RavEngine::Entity> cameraEntity;
	Ref<RavEngine::Transform> trans;
	decimalType dt = 0;
	decimalType movementSpeed = 0.3;
	decimalType sensitivity = 0.1;

	decimalType scaleMovement(decimalType f) {
		return f * dt * movementSpeed;
	}

	decimalType scaleRotation(decimalType f) {
		return glm::radians(sensitivity * dt * f);
	}

	void Start() override {
		Ref<RavEngine::Entity> owner(getOwner());
		trans = transform();
		Ref<RavEngine::World>(GetWorld())->Spawn(cameraEntity);
	}

	void MoveForward(float amt) {
		trans->LocalTranslateDelta(scaleMovement(amt) * trans->Forward());
	}
	void MoveRight(float amt) {
		trans->LocalTranslateDelta(scaleMovement(amt) * trans->Right());
	}

	void MoveUp(float amt) {
		trans->LocalTranslateDelta(scaleMovement(amt) * trans->Up());
	}

	void LookUp(float amt) {
		cameraEntity->transform()->LocalRotateDelta(vector3(scaleRotation(amt), 0, 0));
	}
	void LookRight(float amt) {
		trans->LocalRotateDelta(quaternion(vector3(0, scaleRotation(amt), 0)));
	}

	virtual void Tick(float scale) {
		dt = scale;
	}
};

class PlayerActor : public RavEngine::Entity, public RavEngine::IInputListener {
public:
	Ref<PlayerScript> script;
	PlayerActor() : Entity() {
		script = AddComponent<PlayerScript>(new PlayerScript());
	}

	void Start() override {
		//create a child entity for the camera
		auto cameraEntity = new Entity();
		auto cam = cameraEntity->AddComponent<RavEngine::CameraComponent>(new RavEngine::CameraComponent());
		script->cameraEntity = cameraEntity;

		//set the active camera
		cam->setActive(true);

		transform()->AddChild(cameraEntity->transform());
	}

	virtual ~PlayerActor(){}
};