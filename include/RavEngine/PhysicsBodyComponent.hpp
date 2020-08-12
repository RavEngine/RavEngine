#pragma once
#include "PhysXDefines.h"
#include "Component.hpp"
#include <PxRigidBody.h>
#include <PxRigidDynamic.h>
#include <PxRigidStatic.h>
#include <iostream>
#include <functional>
#include <eventpp/eventdispatcher.h>
#include "mathtypes.hpp"

namespace RavEngine {
	class PhysicsBodyComponent : public Component
	{
	public:
		physx::PxRigidActor* rigidActor = nullptr;
		physx::PxU32 filterGroup = -1;
		physx::PxU32 filterMask = -1;

		virtual ~PhysicsBodyComponent() {}
		virtual vector3 getPos() = 0;
		virtual quaternion getRot() = 0;
		virtual void setPos(const vector3&) = 0;
		virtual void setRot(const quaternion&) = 0;

		void OnColliderEnter(PhysicsBodyComponent* other);

		void OnColliderPersist(PhysicsBodyComponent* other);

		void OnColliderExit(PhysicsBodyComponent* other);
	};

	class RigidBodyDynamicComponent : public PhysicsBodyComponent {
	public:
		RigidBodyDynamicComponent();
		RigidBodyDynamicComponent(physx::PxU32 fg, physx::PxU32 fm) : RigidBodyDynamicComponent() {
			this->filterGroup = fg; this->filterMask = fm;
		}
		virtual ~RigidBodyDynamicComponent();
		vector3 getPos() override;
		quaternion getRot() override;
		virtual void setPos(const vector3&) override;
		virtual void setRot(const quaternion&) override;

		void RegisterAllAlternateTypes() override {
			RegisterAlternateQueryType<PhysicsBodyComponent>();
		}
	};

	//class RigidBodyStatic : public PhysicsBody {
	//protected:
	//	physx::PxRigidStatic* rigidStatic;
	//public:
	//	virtual ~RigidBodyStatic();
	//	Vector3 getPos() override;
	//	Quaternion getRot() override;
	//	virtual void setPos(const Vector3&);
	//	virtual void setRot(const Quaternion&);
	//};

	//soft body?

}