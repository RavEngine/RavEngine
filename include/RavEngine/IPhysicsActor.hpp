#pragma once
#include "WeakRef.hpp"
#include <phmap.h>
#include "SharedObject.hpp"
/**
Multi-inherit from this interface to provide delegates for collision events
*/
namespace RavEngine {
	class PhysicsBodyComponent;
	struct ContactPairPoint;

	struct IPhysicsActor : public virtual_enable_shared_from_this<IPhysicsActor>{
		/**
		Called by a PhysicsBodyComponent when it has collided with another. Override in subclasses.
		@param other the other component
		@param contactPoints the contact point data. Do not hold onto this pointer, because after this call, the pointer is invalid.
		@param numContactPoints the number of contact points. Set to 0 if wantsContactData is false.
		*/
		virtual void OnColliderEnter(const WeakRef<RavEngine::PhysicsBodyComponent>& other, const ContactPairPoint* contactPoints, size_t numContactPoints) {}

		/**
		Called by a PhysicsBodyComponent when it has exited collision with another. Override in subclasses.
		@param other the other component
		@param contactPoints the contact point data. Do not hold onto this pointer, because after this call, the pointer is invalid.
		@param numContactPoints the number of contact points. Set to 0 if wantsContactData is false.
		*/
		virtual void OnColliderExit(const WeakRef<RavEngine::PhysicsBodyComponent>& other, const ContactPairPoint* contactPoints, size_t numContactPoints) {}

		/**
		Called by a PhysicsBodyComponent when it has collided with another and the collision has persisted. Override in subclasses.
		@param other the other component
		@param contactPoints the contact point data. Do not hold onto this pointer, because after this call, the pointer is invalid.
		@param numContactPoints the number of contact points. Set to 0 if wantsContactData is false.
		*/
		virtual void OnColliderPersist(const WeakRef<RavEngine::PhysicsBodyComponent>& other, const ContactPairPoint* contactPoints, size_t numContactPoints) {}
		
		/**
		 Called by a PhysicsBodyComponent when it has entered another trigger . Override in subclasses. Note that triggers cannot fire events on other triggers.
		 @param other the other component
		 */
		virtual void OnTriggerEnter(const WeakRef<RavEngine::PhysicsBodyComponent>& other){}
		
		/**
		 Called by a PhysicsBodyComponent when it has exited another trigger . Override in subclasses. Note that triggers cannot fire events on other triggers.
		 @param other the other component
		 */
		virtual void OnTriggerExit(const WeakRef<RavEngine::PhysicsBodyComponent>& other){}

		virtual size_t Hash() {
			return reinterpret_cast<size_t>(this);
		}

		void OnRegisterBody(const WeakRef<PhysicsBodyComponent>&);
		void OnUnregisterBody(const WeakRef<PhysicsBodyComponent>&);

		~IPhysicsActor();
	private:
		phmap::flat_hash_set<WeakPtrKey<RavEngine::PhysicsBodyComponent>> senders;
	};
}
