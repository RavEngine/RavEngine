#pragma once
#include "Entity.hpp"

template<typename T>
class ComponentHandle{
    Entity owner;
public:
    ComponentHandle(decltype(owner) owner) : owner(owner){}
    
    inline T* operator->(){
        assert(EntityIsValid(owner.id));
        assert(owner.HasComponent<T>());
        return &owner.GetComponent<T>();
    }
};
