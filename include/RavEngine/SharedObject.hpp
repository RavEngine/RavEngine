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
#include <phmap.h>
#include <plf_colony.h>

namespace RavEngine {
	class SharedObject {
		std::atomic<int> refcount = 0;
		SpinLock lock;
		
		plf::colony<WeakRefBase*> weakptrs;
		//std::unordered_set<WeakRefBase*> weakptrs;
	public:
		virtual ~SharedObject() {
			//notify all tracked WeakRefs
			//so that WeakReferences know that their pointers are invalid
			lock.lock();
			for(const auto& ptr : weakptrs){
				ptr->notifyDangling();
			}
			lock.unlock();
		}

		/**
		 Increment the reference count
		 */
		void retain() {
			++refcount;
		}
		/**
		 Decrement the reference count. If the reference count hits 0, the object is destroyed.
		 */
		void release() {
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
		bool operator==(const SharedObject* other) const {
			return reinterpret_cast<uintptr_t>(this) == reinterpret_cast<uintptr_t>(other);;
		}
		
		/**
		 Invoked by WeakRef<T> when they track a new sharedobject.
		 */
		void TrackWeak(WeakRefBase* weakptr){
				lock.lock();
				weakptrs.insert(weakptr);
				lock.unlock();
		}
		
		void UntrackWeak(WeakRefBase* weakptr){
			lock.lock();
			//TODO: optimize by storing iterator and passing it to this method, then erase directly
			plf::colony<WeakRefBase*>::iterator item = weakptrs.begin();
			for( ; item != weakptrs.end(); ++item){
				if (*item == weakptr){
					weakptrs.erase(item);
					break;
				}
			}
			lock.unlock();
		}
	};
}
