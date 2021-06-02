//
//  Entity.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#include "Entity.hpp"
#include "World.hpp"

using namespace RavEngine;
using namespace std;

void Entity::Destroy() {
	assert(IsInWorld());	//if assertion fails, you are trying to remove an entity from a world that is not in that world
	Ref<World>(GetWorld())->Destroy(shared_from_this());
}

RavEngine::Entity::Entity(){
    EmplaceComponent<Transform>();
}
