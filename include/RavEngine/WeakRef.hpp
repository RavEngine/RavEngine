//
//  WeakSharedObjectRef.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

# pragma once
#include <type_traits>
#include <functional>

template<typename T>
using WeakRef = std::weak_ptr<T>;

/**
 For storing WeakRefs in containers
 */
template <class T>
class WeakPtrKey {
    T * m_ptr;              //for checking pointer validity
    std::weak_ptr<T> m_wptr;
public:
    WeakPtrKey() :m_ptr(nullptr) {}
    WeakPtrKey(std::weak_ptr<T> wptr) : m_ptr(wptr.lock().get()), m_wptr(wptr) {}
    
    WeakPtrKey(std::shared_ptr<T> sptr):m_ptr(sptr.get()),m_wptr(sptr) {}
    
    bool operator<(const WeakPtrKey<T> &other) const {
        return m_ptr < other.m_ptr;
        
    }
    
    bool operator==(const WeakPtrKey<T> &other) const {
        return m_ptr == other.m_ptr;
        
    }
    
    std::weak_ptr<T> getWeak() const { return m_wptr;}
    std::shared_ptr<T> lock() const { return m_wptr.lock();}
};
