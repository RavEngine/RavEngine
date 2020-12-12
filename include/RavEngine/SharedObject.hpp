//
//  SharedObject.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//
//

#pragma once

#include <iostream>
#include <atomic>
#include "WeakRef.hpp"
#include <cassert>
#include "Ref.hpp"
#include "SpinLock.hpp"
#include "Locked_Hashmap.hpp"

namespace RavEngine {
	class SharedObject {
		std::atomic<int> refcount = 0;
		
		locked_hashset<WeakRefBase*,SpinLock> weakptrs;
	public:
		virtual ~SharedObject() {
			//notify all tracked WeakRefs
			//so that WeakReferences know that their pointers are invalid
			for(const auto& ptr : weakptrs){
				ptr->notifyDangling();
			}
		}

		/**
		 Increment the reference count
		 */
		inline void retain() {
			++refcount;
		}
		/**
		 Decrement the reference count. If the reference count hits 0, the object is destroyed.
		 */
		inline void release() {
			--refcount;
			if (refcount == 0) {
				delete this;
				return;
			}
		}

		 /**
		 The default hash function. Uses the pointer value, but may be overridden in subclasses.
		 This function is used when hashing the ref objects.
		 @return a hash for the current object
		 */
		virtual size_t Hash() {
			return reinterpret_cast<size_t>(this);
		}

		/**
		Equality operator. Compares addresses.
		@param other the other object to compare
		@return true if the object addresses match.
		*/
		inline bool operator==(const SharedObject* other) const {
			return reinterpret_cast<uintptr_t>(this) == reinterpret_cast<uintptr_t>(other);;
		}
		
		/**
		 Invoked by WeakRef<T> when they track a new sharedobject.
		 */
		inline void TrackWeak(WeakRefBase* weakptr){
				weakptrs.insert(weakptr);
		}
		
		inline void UntrackWeak(WeakRefBase* weakptr){
			weakptrs.erase(weakptr);
		}
	};
}
