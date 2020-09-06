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

namespace RavEngine {
	class System : public SharedObject {
	public:
		//for sharedobject
		virtual ~System() {}

		//query method

		//dependency method
		virtual std::list<std::type_index> QueryTypes() const = 0;

		/**
		Override in subclasses to determine execution order.
		@param other the other system to compare against.
		@return true if this system must run before the other system, false if the other system does not depend on this system.
		*/
		virtual bool MustRunBefore(const std::type_index& other) const{
			return false;
		}

		
		bool operator<(const System& other) const {
			return MustRunBefore(std::type_index(typeid(other)));
		}

		/**
		 Tick the System on an Entity.
		 @param fpsScale the frame rate scale factor computed by the World.
		 @param e the Entity to operate on
		 */
		virtual void Tick(float fpsScale, Ref<Entity> e) const = 0;
	};
}