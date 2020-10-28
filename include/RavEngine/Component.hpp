//
//  Component.h
//  MacFramework
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once
#include "SharedObject.hpp"
#include <typeindex>
#include "WeakRef.hpp"

namespace utils {
	class Entity;
}

namespace RavEngine {
	class Entity;
	class Component : public SharedObject {
	protected:
		WeakRef<Entity> owner;		//non-owning pointer to the owning Entity of this component

		/**
		 * Override in base classes to set the query dynamic types of this Component
		 */
		virtual void RegisterAllAlternateTypes() {}

	public:
		bool Enabled = true;

		//ensure the base class constructor is called if the constructor is overridden
		Component() {
			RegisterAllAlternateTypes();
		}

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

		//for SharedObject
		virtual ~Component() {
		}

		/**
		 Get the object of this component.
		 @return a non-owning pointer to this component's owner. Do not store!
		 */
		WeakRef<Entity> getOwner() const {
			return owner;	//creates a non-owning pointer
		}

		/**
		 Set the owner of this component. This is stored in a non-owning fashion.
		 @param newOwner the pointer to the new owner of the component
		 */
		void SetOwner(WeakRef<Entity> newOwner) {
			owner = newOwner;
		}

		//define values in subclass...
	};
}
