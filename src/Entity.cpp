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

Ref<TransformComponent> Entity::transform(){
	return components.GetComponent<TransformComponent>();
}


Entity::Entity(){
	AddComponent<TransformComponent>(new TransformComponent());
}

void Entity::Draw() {
	//get the material and draw
	if (components.HasComponentOfType<StaticMesh>()) {
		components.GetComponent<StaticMesh>()->Draw();
	}
}

Entity::~Entity(){
    components.clear();
    worldptr = nullptr;
}
