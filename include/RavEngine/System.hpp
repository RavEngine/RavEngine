#pragma once
#include "CTTI.hpp"
#include <vector>

namespace RavEngine {
	class World;

	class System {
	public:
		typedef std::vector<ctti_t> list_type;


		/**
		Override in subclasses to determine execution order.
		@param other the other system to compare against.
		@return true if this system must run before the other system, false if the other system does not depend on this system.
		*/
//		virtual const list_type& MustRunBefore() const{
//			return empty;
//		}
//
		/**
		 Override in subclasses to determine execution order.
		 @param other the other system to compare against.
		 @return true if this system must run after the other system, false otherwise
		 */
//		virtual const list_type& MustRunAfter() const{
//			return empty;
//		}

	protected:
	};
}
