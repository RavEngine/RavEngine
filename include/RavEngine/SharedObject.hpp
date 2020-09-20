//
//  SharedObject.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//
// 	Inherit from this class to enable automatic reference counting
//	memory management. Be sure to update the ref. Destructor must be virtual and public!
//

#pragma once

#include <iostream>
#include <atomic>
#include "WeakRef.hpp"
#include <cassert>
#include "Ref.hpp"
#include "SpinLock.hpp"
#include "AppEnd.h"
#include <list>

namespace RavEngine {
	class SharedObject {
		std::atomic<int> refcount = 0;
		SpinLock lock;
		
		// for the hashmap - prevents investigating objects and instead only uses the addresses
//		struct WeakHasherByPtr{
//			size_t operator()(const WeakRefBase* ptr) const{
//				return reinterpret_cast<size_t>(ptr);
//			}
//		};
//
//		struct WeakCompareByPtr{
//			bool operator()(const WeakRefBase* ptr1, const WeakRefBase* ptr2) const{
//				return reinterpret_cast<uintptr_t>(ptr1) == reinterpret_cast<uintptr_t>(ptr2);
//			}
//		};
		
		std::list<WeakRefBase*> weakptrs;
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
			if(!RAVENGINE_ATEXIT){
				lock.lock();
				weakptrs.push_back(weakptr);
				lock.unlock();
			}
		}
		
		void UntrackWeak(WeakRefBase* weakptr){
			if(!RAVENGINE_ATEXIT){
				lock.lock();
				weakptrs.remove(weakptr);
				lock.unlock();
			}
		}
	};
}
