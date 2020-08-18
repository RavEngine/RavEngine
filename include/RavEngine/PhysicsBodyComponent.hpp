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
		virtual vector3 getPos();
		virtual quaternion getRot();
		virtual void setPos(const vector3&);
		virtual void setRot(const quaternion&);

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

		void RegisterAllAlternateTypes() override {
			RegisterAlternateQueryType<PhysicsBodyComponent>();
		}
	};

	class RigidBodyStaticComponent : public PhysicsBodyComponent {
	public:
		RigidBodyStaticComponent();
		virtual ~RigidBodyStaticComponent();
		RigidBodyStaticComponent(physx::PxU32 fg, physx::PxU32 fm) : RigidBodyStaticComponent() {
			this->filterGroup = fg; this->filterMask = fm;
		}

		void RegisterAllAlternateTypes() override {
			RegisterAlternateQueryType<PhysicsBodyComponent>();
		}
	};
}