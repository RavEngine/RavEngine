//
//  WeakSharedObjectRef.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

# pragma once
#include <memory>

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
    
    inline bool operator<(const WeakPtrKey<T> &other) const {
        return m_ptr < other.m_ptr;
        
    }
    
    inline bool operator==(const WeakPtrKey<T> &other) const {
        return m_ptr == other.m_ptr;
        
    }
    
    /**
     Hash the pointer value of the original address
     @return hash code for this object
     */
    inline std::size_t hash() const{
        return reinterpret_cast<std::size_t>(m_ptr);
    }
    
    inline std::weak_ptr<T> get_weak() const { return m_wptr;}
    inline std::shared_ptr<T> lock() const { return m_wptr.lock();}
};

namespace std{
template<typename T>
struct hash<WeakPtrKey<T>>{
    inline std::size_t operator()(const WeakPtrKey<T>& ptr) const{
        return ptr.hash();
    }
};
}
