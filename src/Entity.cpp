//
//  Entity.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#include "Entity.hpp"
#include "Transform.hpp"

using namespace RavEngine;
Transform& Entity::GetTransform(){
    return GetComponent<Transform>();
}
