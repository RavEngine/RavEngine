//
//  Component.h
//  MacFramework
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once
#include "SharedObject.hpp"
#include <list>
#include <typeindex>
#include "WeakRef.hpp"

//forward declarations
class Entity;

class Component : public SharedObject{
protected:
	WeakRef<Entity> owner;		//non-owning pointer to the owning Entity of this component
	typedef std::list<std::type_index> alternateTypeStore;
	alternateTypeStore alternateTypeRegister;

	/**
	Make the Engine aware of different types this Component may be queried as.
	Do not register the most derived type, that is done automatically.
	*/
	template<typename T>
	void RegisterAlternateQueryType(){
		static_assert(std::is_base_of<Component, T>::value, "Must be a subclass of Component!");
		//static_assert(std::is_convertible<ref, Ref<Component>>::value, "Must be a Component Reference");
		alternateTypeRegister.push_back(std::type_index(typeid(T)));
	}

	/**
	 * Override in base classes to set the query dynamic types of this Component
	 */
	virtual void RegisterAllAlternateTypes(){}

public:
	bool Enabled = true;

	//ensure the base class constructor is called if the constructor is overridden
	Component() {
		RegisterAllAlternateTypes();
	}
	
	/**
	 * Called by the parent entity after it is added. Override in subclasses
	 * @param e the parent entity invoking the call
	 */
	virtual void AddHook(const WeakRef<Entity>& e) {}

	/*
	 * Get the list of alternate types this component may be queried as
	 */
	const alternateTypeStore& getAltTypes() {
		return alternateTypeRegister;
	}

	//for SharedObject
	virtual ~Component(){
	}
	
	/**
	 Get the object of this component.
	 @return a non-owning pointer to this component's owner. Do not store!
	 */
	WeakRef<Entity> getOwner() const{
		return owner;	//creates a non-owning pointer
	}
	
	/**
	 Set the owner of this component. This is stored in a non-owning fashion.
	 @param newOwner the pointer to the new owner of the component
	 */
	void SetOwner(WeakRef<Entity> newOwner){
		owner = newOwner;
	}
	
	//define values in subclass...
};

//macro for checking type in template
#define C_REF_CHECK static_assert(std::is_base_of<Component, T>::value, "Template parameter must be a Component subclass");