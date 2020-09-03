#pragma once
#include "PhysXDefines.h"
#include "Component.hpp"
#include <PxRigidBody.h>
#include <PxRigidDynamic.h>
#include <PxRigidStatic.h>
#include <iostream>
#include <functional>
#include "eventpp/eventdispatcher.h"
#include "mathtypes.hpp"
#include "IPhysicsActor.hpp"
#include <unordered_set>

namespace RavEngine {
	class PhysicsBodyComponent : public Component
	{
	protected:
		std::unordered_set<IPhysicsActor*> receivers;
	public:
		physx::PxRigidActor* rigidActor = nullptr;
		physx::PxU32 filterGroup = -1;
		physx::PxU32 filterMask = -1;

		/**
		Add a recipient for collision events. Must implement IPhysicsActor.
		@param obj the interface implementer to recieve the events
		*/
		void AddReceiver(IPhysicsActor* obj);
		/**
		Remove a recipient for collision events. Must implement IPhysicsActor On deallocation, objects automatically remove themselves.
		@param obj the object to remove
		*/
		void RemoveReceiver(IPhysicsActor* obj);

		virtual ~PhysicsBodyComponent();
		virtual vector3 getPos();
		virtual quaternion getRot();
		virtual void setPos(const vector3&);
		virtual void setRot(const quaternion&);

		void SetGravityEnabled(bool);

		/**
		@returns true if gravity is enabled
		*/
		bool GetGravityEnabled();

		void SetSleepNotificationsEnabled(bool);

		/**
		Returns true if sleep / wake notifications are enabled.
		*/
		bool GetSleepNotificationsEnabled();

		void SetSimulationEnabled(bool);

		/**
		@returns true if simulation is enabled.
		*/
		bool GetSimulationEnabled();

		/**
		Invoked when a collider begins colliding with another body
		@param other the second body
		*/
		void OnColliderEnter(PhysicsBodyComponent* other);

		/**
		Invoked when a collider has collided with another body for multiple frames
		@param other the second body
		*/
		void OnColliderPersist(PhysicsBodyComponent* other);

		/**
		Invoked when a collider has exited another collider
		@param other the second body
		*/
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
		
		/**
		@returns the body's current linear velocity
		*/
		vector3 GetLinearVelocity();

		/**
		@returns the current body's angular velocity in euler angles
		*/
		vector3 GetAngularVelocity();

		void SetLinearVelocity(const vector3&, bool);

		void SetAngularVelocity(const vector3&, bool);

		/**
		Wake the body
		*/
		void Wake();

		/**
		Put the body to sleep
		*/
		void Sleep();

		/**
		@return true if the body is asleep.
		*/
		bool IsSleeping();
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