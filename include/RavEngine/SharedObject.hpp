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

class SharedObject{
	std::atomic<int> refcount = 0;
public:
	virtual ~SharedObject(){
        //so that WeakReferences know that their pointers are invalid
        WeakRefBase::Remove(this);
    }
	
	/**
	 Increment the reference count
	 */
	void retain(){
		++refcount;
	}
	/**
	 Decrement the reference count. If the reference count hits 0, the object is destroyed.
	 */
	void release(){
		--refcount;
		if (refcount == 0){
			delete this;
			return;
		}
	}
    
    /**
     Get a safe non-owning reference to the current object
     @return a WeakReference to this object
     */
    /*WeakSharedObjectRef makeWeak(){
        return WeakSharedObjectRef(this);
    }*/

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
	bool operator==(const SharedObject* other) const{
		return reinterpret_cast<uintptr_t>(this) == reinterpret_cast<uintptr_t>(other);;
	}
};

