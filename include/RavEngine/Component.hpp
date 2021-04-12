#pragma once

#include "WeakRef.hpp"
#include "CTTI.hpp"

namespace RavEngine {
	class Entity;
	class Component : public AutoCTTI {
	protected:
		WeakRef<Entity> owner;		//non-owning pointer to the owning Entity of this component
		friend class Entity;
		
		/**
		 * Called by the parent entity after this component is added. Override in subclasses
		 * @param e the parent entity invoking the call
		 */
		virtual void AddHook(const WeakRef<RavEngine::Entity>& e) {}
		
		/**
		 * Called by the parent entity before this component is removed. Override in subclasses
		 * @param e the parent entity invoking the call
		 */
		virtual void RemoveHook(const WeakRef<RavEngine::Entity>& e) {}
	public:
		bool Enabled = true;

		Component() {}

		/**
		 Get the object of this component.
		 @return a non-owning pointer to this component's owner. Do not store!
		 */
		inline WeakRef<Entity> getOwner() const {
			return owner;	//creates a non-owning pointer
		}

		/**
		 Set the owner of this component. This is stored in a non-owning fashion.
		 @param newOwner the pointer to the new owner of the component
		 */
		inline void SetOwner(WeakRef<Entity> newOwner) {
			owner = newOwner;
		}

		//define values in subclass...
	};
}

