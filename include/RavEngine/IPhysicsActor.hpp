#pragma once
#include "WeakRef.hpp"
#include <unordered_set>

/**
Multi-inherit from this interface to provide delegates for collision events
*/
namespace RavEngine {
	class PhysicsBodyComponent;

	struct IPhysicsActor {
		/**
		Called by a PhysicsBodyComponent when it has collided with another. Override in subclasses.
		@param other the other component
		*/
		virtual void OnColliderEnter(const WeakRef<RavEngine::PhysicsBodyComponent>& other) {}

		/**
		Called by a PhysicsBodyComponent when it has exited collision with another. Override in subclasses.
		@param other the other component
		*/
		virtual void OnColliderExit(const WeakRef<RavEngine::PhysicsBodyComponent>& other) {}

		/**
		Called by a PhysicsBodyComponent when it has collided with another and the collision has persisted. Override in subclasses.
		@param other the other component
		*/
		virtual void OnColliderPersist(const WeakRef<RavEngine::PhysicsBodyComponent>& other) {}

		virtual size_t Hash() {
			return reinterpret_cast<size_t>(this);
		}

		void OnRegisterBody(const WeakRef<PhysicsBodyComponent>&);
		void OnUnregisterBody(const WeakRef<PhysicsBodyComponent>&);

		~IPhysicsActor();
	private:
		std::unordered_set<WeakRef<RavEngine::PhysicsBodyComponent>> senders;
	};
}
