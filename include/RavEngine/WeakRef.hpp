//
//  WeakSharedObjectRef.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

# pragma once
#include <type_traits>
#include <functional>

namespace RavEngine {
    class SharedObject;
}

// helper base class to ensure static member is shared, do not use!
class WeakRefBase {
protected:
	/**
	 Update the tracking structure to know about this connection
	 @param obj the pointer to associate this WeakRef with
	 */
	void Associate(RavEngine::SharedObject* obj);
	
	/**
	 Update the tracking structure to forget about this connection
	 @param obj the pointer to dissassociate this WeakRef with
	 */
	void Dissassociate(RavEngine::SharedObject* obj);
	
public:
	virtual void notifyDangling() = 0;
};


//templated forward-declaration
template<typename T>
class Ref;

template<typename T>
class WeakRef : public WeakRefBase {
protected:
    T* ptr;
public:

    //construct from pointer
    WeakRef(T* obj) {
        if (obj == nullptr) {
            ptr = nullptr;
            return;
        }
        static_assert(std::is_base_of<RavEngine::SharedObject, T>::value, "T must derive from SharedObject!");
        //track this object
        Associate(obj);
        ptr = obj;
    }
    //construct from Ref
    WeakRef(const Ref<T>& other) : WeakRef(other.get()) {};

    //copy assignment
    inline WeakRef<T>& operator=(const WeakRef<T>& other) {
        if (&other == this) {
            return *this;
        }

        //dissasociate with old pointer
        if (ptr != nullptr) {
            Dissassociate(reinterpret_cast<RavEngine::SharedObject*>(ptr));
        }
        ptr = other.get();
        //associate with new pointer
        if (ptr != nullptr) {
            Associate(reinterpret_cast<RavEngine::SharedObject*>(ptr));
        }
        else {
            setNull();
        }
        return *this;
    }

    //default constructor
    WeakRef() {
        setNull();
    }

    ~WeakRef() {
        //untrack self
        if (ptr != nullptr) {
            Dissassociate(reinterpret_cast<RavEngine::SharedObject*>(ptr));
            setNull();
        }
    }

    /**
    Return the bare pointer. This should always be immediately converted into a SharedObjectRef owning pointer.
        @return the pointer in this WeakReference, or nullptr if the pointer is not valid
        */
    inline T* get() const {
        return ptr;                 //this will become nullptr if the object was destroyed
    }

    /**
        Check if this reference is a null reference
        @return true if ptr is null
        */
    inline bool isNull() const {
        return ptr == nullptr;
    }

    /**
        Set this weakreference as a null reference
        */
    inline void setNull() {
        ptr = nullptr;
    }

    /**
    Equality operator, compares null status and pointer values
    */
    inline bool operator==(const WeakRef& other) const {
        if (isNull() == other.isNull()) {
            return true;
        }
        //compare pointer values
        return get() == other.get();
    }
	
	/**
	 Shortcut for truthy
	 @return true if this weakref is NOT null, false otherwise
	 */
	inline operator bool() const{
		return !isNull();
	}

    /**
    update because the object this weakpointer points to was deallocated
    */
    inline void notifyDangling() override {
        setNull();
    }
};

//hash function for weak ref
namespace std {
    template<typename T>
    struct hash<WeakRef<T>> {
        //weak pointer hash uses the object it is pointing at, or 0 if is null
        inline std::size_t operator()(const WeakRef<T>& k) const {
            if (k.isNull()) {
                return 0;
            }
            return k.get()->Hash();
        }
    };
}
