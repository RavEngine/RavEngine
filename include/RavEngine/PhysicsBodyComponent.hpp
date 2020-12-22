#pragma once
#include "PhysXDefines.h"
#include "Component.hpp"
#include <PxRigidBody.h>
#include <PxRigidDynamic.h>
#include <PxRigidStatic.h>
#include <iostream>
#include <functional>
#include "mathtypes.hpp"
#include "IPhysicsActor.hpp"
#include <phmap.h>
#include "Queryable.hpp"

namespace RavEngine {
	class PhysicsBodyComponent : public Component, public Queryable<PhysicsBodyComponent>
	{
	protected:
		phmap::flat_hash_set<IPhysicsActor*> receivers;
	public:
		physx::PxRigidActor* rigidActor = nullptr;
		physx::PxU32 filterGroup = -1;
		physx::PxU32 filterMask = -1;

		void AddHook(const WeakRef<RavEngine::Entity>& e);

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
		virtual vector3 getPos() const;
		virtual quaternion getRot() const;
		virtual void setPos(const vector3&);
		virtual void setRot(const quaternion&);

		void SetGravityEnabled(bool);

		/**
		@returns true if gravity is enabled
		*/
		bool GetGravityEnabled() const;

		void SetSleepNotificationsEnabled(bool);

		/**
		Returns true if sleep / wake notifications are enabled.
		*/
		bool GetSleepNotificationsEnabled() const;

		void SetSimulationEnabled(bool);

		/**
		@returns true if simulation is enabled.
		*/
		bool GetSimulationEnabled() const;

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
		
		/**
		 Called by a PhysicsBodyComponent when it has entered another trigger . Override in subclasses. Note that triggers cannot fire events on other triggers.
		 @param other the other component
		 */
		void OnTriggerEnter(PhysicsBodyComponent* other);
		
		/**
		 Called by a PhysicsBodyComponent when it has exited another trigger . Override in subclasses. Note that triggers cannot fire events on other triggers.
		 @param other the other component
		 */
		void OnTriggerExit(PhysicsBodyComponent* other);
	};

	class RigidBodyDynamicComponent : public PhysicsBodyComponent {
	public:
		RigidBodyDynamicComponent();
		RigidBodyDynamicComponent(physx::PxU32 fg, physx::PxU32 fm) : RigidBodyDynamicComponent() {
			this->filterGroup = fg; this->filterMask = fm;
		}
		virtual ~RigidBodyDynamicComponent();
		
		/**
		@returns the body's current linear velocity
		*/
		vector3 GetLinearVelocity() const;

		/**
		@returns the current body's angular velocity in euler angles
		*/
		vector3 GetAngularVelocity() const;

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
		
		enum AxisLock{
			Linear_X = (1 << 0),
			Linear_Y = (1 << 1),
			Linear_Z = (1 << 2),
			Angular_X = (1 << 3),
			Angular_Y = (1 << 4),
			Angular_Z = (1 << 5)
		};
		
		/**
		 Set the axis locking flags.
		 @see AxisLock enum.
		 @param LockFlags a bitmask representing the axes to lock or unlock
		 */
		void SetAxisLock(uint16_t LockFlags);
		
		/**
		 @return the currently-active locking flags
		 @see AxisLock enum
		 */
		uint16_t GetAxisLock() const;
		
		/**
		 Set the mass of this physics body
		 @param mass the mass of the body
		 */
		void SetMass(decimalType mass);
		
		/**
		 @return the mass of the body
		 */
		decimalType GetMass() const;
		
		/**
		 @return the inverse mass of the body
		 */
		decimalType GetMassInverse() const;
		
		/**
		 Add a force to the object
		 @param force the vector representing the force
		 */
		void AddForce(const vector3& force);
		
		/**
		 Add a torque to the object
		 @param torque the vector representing the torque
		 */
		void AddTorque(const vector3& torque);
		
		/**
		 Reset all active forces on the object
		 */
		void ClearAllForces();
		
		/**
		 Reset all active torques on the object
		 */
		void ClearAllTorques();
	};

	class RigidBodyStaticComponent : public PhysicsBodyComponent {
	public:
		RigidBodyStaticComponent();
		virtual ~RigidBodyStaticComponent();
		RigidBodyStaticComponent(physx::PxU32 fg, physx::PxU32 fm) : RigidBodyStaticComponent() {
			this->filterGroup = fg; this->filterMask = fm;
		}
	};
}
