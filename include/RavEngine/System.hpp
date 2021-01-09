//
//  System.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#pragma once

#include "SharedObject.hpp"
#include "Entity.hpp"
#include <plf_list.h>
#include "CTTI.hpp"

namespace RavEngine {
	class World;

	class System : public SharedObject {
	public:
		typedef plf::list<ctti_t> list_type;
		
		//for sharedobject
		virtual ~System() {}

		//query method

		//dependency method
		virtual const list_type& QueryTypes() const = 0;

		/**
		Override in subclasses to determine execution order.
		@param other the other system to compare against.
		@return true if this system must run before the other system, false if the other system does not depend on this system.
		*/
		virtual const list_type& MustRunBefore() const{
			return empty;
		}
		
		/**
		 Override in subclasses to determine execution order.
		 @param other the other system to compare against.
		 @return true if this system must run after the other system, false otherwise
		 */
		virtual const list_type& MustRunAfter() const{
			return empty;
		}
		
		/**
		 Override in subclasses. Used for dependency sorting.
		 @return this System's static ID (call CTTI<T>)
		 */
		virtual ctti_t ID() const = 0;

		/**
		 Tick the System on an Entity.
		 @param fpsScale the frame rate scale factor computed by the World.
		 @param e the Entity to operate on
		 */
		virtual void Tick(float fpsScale, Ref<Entity> e) = 0;
		
	protected:
		static const list_type empty;
	};
}
