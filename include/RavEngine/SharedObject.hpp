#pragma once
//
//  SharedObject.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//
//


#include "WeakRef.hpp"
#include "Ref.hpp"

namespace RavEngine {
	/**
	 A workaround class to multi-inheritance with enable_shared_from_this.
	 Don't inherit from this class.
	 */
	struct virtual_enable_shared_from_this_base : std::enable_shared_from_this<virtual_enable_shared_from_this_base> {
		virtual ~virtual_enable_shared_from_this_base() {}
	};
	
/**
	 A workaround class to multi-inheritance with enable_shared_from_this.
	 Inherit non-virtually.
	 */
	template<typename T>
	struct virtual_enable_shared_from_this : virtual virtual_enable_shared_from_this_base {
		std::shared_ptr<T> shared_from_this() {
			return std::dynamic_pointer_cast<T>(virtual_enable_shared_from_this_base::shared_from_this());
		}
	};

}
