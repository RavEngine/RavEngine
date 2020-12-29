//
//  Entity.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#include "Entity.hpp"
#include "System.hpp"
#include "StaticMesh.hpp"
#include "World.hpp"

using namespace glm;
using namespace RavEngine;

void Entity::Destroy() {
	Ref<World>(GetWorld())->Destroy(this);
}

RavEngine::Entity::Entity(){
	AddComponent<Transform>(new Transform());
}

RavEngine::Entity::~Entity(){
    components.clear();
}
