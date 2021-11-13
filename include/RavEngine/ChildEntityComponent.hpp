#pragma once

#include "Queryable.hpp"
#include "Entity.hpp"

namespace RavEngine{
/**
 Create an entity association. When the owner entity is spawned or destroyed, this entity will also be spawned / destroyed with it.
 Allows prefab-like design.
 */
class ChildEntityComponent : public Queryable<ChildEntityComponent>, public AutoCTTI{
protected:
    Entity child;
public:
	ChildEntityComponent(decltype(child) e) : child(e){}
	
	decltype(child) GetEntity()const{
		return child;
	}

};
}
