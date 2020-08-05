//
//  System.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once

#include "SharedObject.hpp"
#include "Entity.hpp"

class World;

class System : public SharedObject{
public:
	//for sharedobject
	virtual ~System(){}
	
	/**
	 Tick the System on an Entity.
	 @param fpsScale the frame rate scale factor computed by the World.
	 @param e the Entity to operate on
	 */
	virtual void Tick(float fpsScale,Ref<Entity> e) const = 0;
};