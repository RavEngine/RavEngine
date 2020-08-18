//
//  Entity.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#include "Entity.hpp"
#include "System.hpp"
#include "StaticMesh.hpp"
#include "World.hpp"

using namespace glm;
using namespace RavEngine;

Ref<Transform> RavEngine::Entity::transform(){
	return components.GetComponent<Transform>();
}

void Entity::Destroy() {
	Ref<World>(GetWorld())->Destroy(this);
}

RavEngine::Entity::Entity(){
	AddComponent<Transform>(new Transform());
}

RavEngine::Entity::~Entity(){
    components.clear();
    worldptr = nullptr;
}
