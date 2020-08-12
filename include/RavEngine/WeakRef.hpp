//
//  WeakSharedObjectRef.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

# pragma once
#include <unordered_set>
#include <unordered_map>
#include <mutex>

namespace RavEngine {
    class SharedObject;
}

// helper base class to ensure static member is shared, do not use!
class WeakRefBase {
protected:
    static std::mutex mtx;
    //this tracks all SharedObjects for weak pointer validity
    //the first address is the address of the tracked object, and the second tracks the addresses of all weak pointers referring to it
    typedef std::unordered_map<void*, std::unordered_set<WeakRefBase*>> TrackedPtrStore;
    static TrackedPtrStore WeakReferences;

    virtual void notifyDangling() = 0;
public:
    /**
    Remove a SharedObject from the WeakReferences map. This should only be done in SharedObject destructors.
        @param obj the object to remove
        */
    static void Remove(RavEngine::SharedObject* obj) {
        //only act if the structure is tracking this pointer
        auto addr = obj;
        mtx.lock();
        auto found = WeakReferences.find(addr);
        if (found != WeakReferences.end()) {
            const auto& set = WeakReferences.at(addr);
            for (const auto& wr : set) {
                //signal that the weak object's pointer has been invalidated
                wr->notifyDangling();
            }
            //remove from the map
            WeakReferences.erase(found);
        }
        mtx.unlock();
    }

};


//templated forward-declaration
template<typename T>
class Ref;

template<typename T>
class WeakRef : public WeakRefBase {
protected:
    T* ptr;

    /**
        Update the tracking structure to know about this connection
        @param obj the pointer to associate this WeakRef with
        */
    void Associate(T* obj) {
        if (ptr == nullptr) {
            return;
        }
        mtx.lock();
        WeakReferences[obj].insert(this);
        mtx.unlock();
    }

    /**
    Update the tracking structure to forget about this connection
    @param obj the pointer to dissassociate this WeakRef with
    */
    void Dissassociate(T* obj) {
        if (ptr == nullptr) {
            return;
        }
        mtx.lock();
        WeakReferences[obj].erase(this);
        mtx.unlock();
    }
public:

    //construct from pointer
    WeakRef(T* obj) {
        if (obj == nullptr) {
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
    WeakRef<T>& operator=(const WeakRef<T>& other) {
        if (&other == this) {
            return *this;
        }

        //dissasociate with old pointer
        Dissassociate(ptr);
        ptr = other.get();
        //associate with new pointer
        Associate(ptr);
        return *this;
    }

    //default constructor
    WeakRef() {
        setNull();
    }

    ~WeakRef() {
        //untrack self
        if (ptr != nullptr) {
            Dissassociate(ptr);
            setNull();
        }
    }

    /**
    Return the bare pointer. This should always be immediately converted into a SharedObjectRef owning pointer.
        @return the pointer in this WeakReference, or nullptr if the pointer is not valid
        */
    T* get() const {
        return ptr;                 //this will become nullptr if the object was destroyed
    }

    /**
        Check if this reference is a null reference
        @return true if ptr is null
        */
    bool isNull() const {
        return ptr == nullptr;
    }

    /**
        Set this weakreference as a null reference
        */
    void setNull() {
        ptr = nullptr;
    }

    /**
    Equality operator, compares null status and pointer values
    */
    bool operator==(const WeakRef& other) const {
        if (isNull() == other.isNull()) {
            return true;
        }
        //compare pointer values
        return get() == other.get();
    }

    /**
    update because the object this weakpointer points to was deallocated
    */
    void notifyDangling() override {
        setNull();
    }
};

//hash function for weak ref
namespace std {
    template<typename T>
    struct hash<WeakRef<T>> {
        //weak pointer hash uses the object it is pointing at, or 0 if is null
        std::size_t operator()(const WeakRef<T>& k) const {
            if (k.isNull()) {
                return 0;
            }
            return k.get()->Hash();
        }
    };
}