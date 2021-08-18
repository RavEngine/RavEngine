#pragma once

#include "Component.hpp"
#include "Entity.hpp"
#include "Queryable.hpp"

namespace RavEngine{
/**
 Create an entity association. When the owner entity is spawned or destroyed, this entity will also be spawned / destroyed with it.
 Allows prefab-like design.
 */
class ChildEntityComponent : public Component, public Queryable<ChildEntityComponent>{
public:
	ChildEntityComponent(Ref<Entity> e) : child(e){}
	
	Ref<Entity> GetEntity()const{
		return child;
	}
	
protected:
	Ref<Entity> child;
};
}
