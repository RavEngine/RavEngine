#pragma once
#include "PhysXDefines.h"
#include "Component.hpp"
#include <PxRigidBody.h>
#include <PxRigidDynamic.h>
#include <PxRigidStatic.h>
#include <PxScene.h>
#include "Function.hpp"
#include "mathtypes.hpp"
#include "PhysicsCallback.hpp"
#include <phmap.h>
#include "Queryable.hpp"
#include <PxSimulationEventCallback.h>
#include "ComponentWithOwner.hpp"
#include <boost/poly_collection/base_collection.hpp>

namespace RavEngine {
	// stores contact points
	struct ContactPairPoint {
		vector3 position, normal, impulse;
		decimalType separation;

		ContactPairPoint(const physx::PxContactPairPoint& pxcpp) :
			position(pxcpp.position.x, pxcpp.position.y, pxcpp.position.z),
			normal(pxcpp.normal.x, pxcpp.normal.y, pxcpp.normal.z),
			impulse(pxcpp.impulse.x, pxcpp.impulse.y, pxcpp.impulse.z),
			separation(pxcpp.separation) {}
		
		// default constructor
		ContactPairPoint(){}
	};

	class PhysicsBodyComponent : public ComponentWithOwner, public Queryable<PhysicsBodyComponent>
	{
	protected:
        UnorderedSet<std::shared_ptr<PhysicsCallback>> receivers;
        boost::base_collection<PhysicsCollider> colliders;
        void CompleteConstruction();
	public:
		physx::PxRigidActor* rigidActor = nullptr;
		physx::PxU32 filterGroup = -1;
		physx::PxU32 filterMask = -1;
        
        PhysicsBodyComponent(entity_t owner);
        virtual ~PhysicsBodyComponent();
        
        void OnDestroy();
        
        template<typename T>
        struct ColliderHandle{
            void* id;
        };
        
        template<typename T, typename ... A>
        ColliderHandle<T> EmplaceCollider(A ... args){
            auto collider_iter = colliders.emplace<T>(this,args...);
            
            return ColliderHandle<T>{static_cast<void*>((*collider_iter).collider)};
        }
        
        template<typename T>
        bool DestroyCollider(ColliderHandle<T> handle){
            for(const auto& collider : colliders){
                if (collider.collider == handle.id){
                    //
                    static_cast<T*>(handle.id)->Destroy();
                    colliders.erase(handle);
                    return true;
                }
            }
            return false;
        }
        
        template<typename T>
        T& GetColliderForHandle(ColliderHandle<T> handle){
            for(const auto& collider : colliders){
                if (collider.collider == handle.id){
                    return static_cast<T>(collider);
                }
            }
            // not found, this is an invalid use
            Debug::Fatal("Component with ID {} not found",handle.id);
        }
        
		/**
		Add a recipient for collision events. Must implement IPhysicsActor.
		@param obj the interface implementer to recieve the events
		*/
		void AddReceiver(decltype(receivers)::value_type& obj);

		/**
		Remove a recipient for collision events. Must implement IPhysicsActor On deallocation, objects automatically remove themselves.
		@param obj the object to remove
		*/
		void RemoveReceiver(decltype(receivers)::value_type& obj);
        
        void RemoveReceiver(PhysicsCallback*);

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
		@param contactPoints the contact point data. Do not hold onto this pointer, because after this call, the pointer is invalid.
		@param numContactPoints the number of contact points. Set to 0 if wantsContactData is false.
		*/
		void OnColliderEnter(PhysicsBodyComponent& other, const ContactPairPoint* contactPoints, size_t numContactPoints);

		/**
		Invoked when a collider has collided with another body for multiple frames
		@param other the second body
		@param contactPoints the contact point data. Do not hold onto this pointer, because after this call, the pointer is invalid.
		@param numContactPoints the number of contact points. Set to 0 if wantsContactData is false.
		*/
		void OnColliderPersist(PhysicsBodyComponent& other, const ContactPairPoint* contactPoints, size_t numContactPoints);

		/**
		Invoked when a collider has exited another collider
		@param other the second body
		@param contactPoints the contact point data. Do not hold onto this pointer, because after this call, the pointer is invalid.
		@param numContactPoints the number of contact points. Set to 0 if wantsContactData is false.
		*/
		void OnColliderExit(PhysicsBodyComponent& other, const ContactPairPoint* contactPoints, size_t numContactPoints);
		
		/**
		 Called by a PhysicsBodyComponent when it has entered another trigger. Override in subclasses. Note that triggers cannot fire events on other triggers.
		 @param other the other component
		 */
		void OnTriggerEnter(PhysicsBodyComponent& other);
		
		/**
		 Called by a PhysicsBodyComponent when it has exited another trigger. Override in subclasses. Note that triggers cannot fire events on other triggers.
		 @param other the other component
		 */
		void OnTriggerExit(PhysicsBodyComponent& other);

		/**
		* Get if this body wants contact data
		*/
		inline bool GetWantsContactData() const {
			return wantsContactData;
		}
		/**
		wantsContactData controls if the simulation calculates and extracts contact point information on collisions.
		Set to true to get contact point data. If set to false, OnCollider functions will have a dangling contactPoints pointer and numContactPoints will be 0.
		*/
		inline void SetWantsContactData(bool state) {
			wantsContactData = state;
		}
	protected:
		bool wantsContactData = false;

		template<typename T>
		inline void LockWrite(const T& func){
            if (rigidActor->getScene()){
                rigidActor->getScene()->lockWrite();
                func();
                rigidActor->getScene()->unlockWrite();
            }
            else{
                func();
            }
		}
        
        template<typename T>
        inline void LockRead(const T& func) const{
            if(rigidActor->getScene()){
                rigidActor->getScene()->lockRead();
                func();
                rigidActor->getScene()->unlockRead();
            }
            else{
                func();
            }
        }
	};

	class RigidBodyDynamicComponent : public PhysicsBodyComponent, public QueryableDelta<PhysicsBodyComponent,RigidBodyDynamicComponent> {
	public:
		RigidBodyDynamicComponent(entity_t owner);
		RigidBodyDynamicComponent(entity_t owner, physx::PxU32 fg, physx::PxU32 fm) : RigidBodyDynamicComponent(owner) {
			this->filterGroup = fg; this->filterMask = fm;
		}
		virtual ~RigidBodyDynamicComponent();
        using QueryableDelta<PhysicsBodyComponent,RigidBodyDynamicComponent>::GetQueryTypes;
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

		// call underlying
		void OnDestroy() {
			PhysicsBodyComponent::OnDestroy();
		}
	};

	struct RigidBodyStaticComponent : public PhysicsBodyComponent, public QueryableDelta<PhysicsBodyComponent,RigidBodyStaticComponent> {
        using QueryableDelta<PhysicsBodyComponent,RigidBodyStaticComponent>::GetQueryTypes;
		RigidBodyStaticComponent(entity_t owner);
		virtual ~RigidBodyStaticComponent();
		RigidBodyStaticComponent(entity_t owner, physx::PxU32 fg, physx::PxU32 fm) : RigidBodyStaticComponent(owner) {
			this->filterGroup = fg; this->filterMask = fm;
		}

		// call underlying
		void OnDestroy() {
			PhysicsBodyComponent::OnDestroy();
		}
	};
}
