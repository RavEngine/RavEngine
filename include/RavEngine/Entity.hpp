#pragma once
#include "Registry.hpp"

namespace RavEngine{
struct World;
struct Transform;

struct Entity{
    entity_t id = INVALID_ENTITY;
    
    Entity(entity_t id) : id(id){}
    Entity(){}
    
    template<typename T, typename ... A>
    inline T& EmplaceComponent(A ... args){
        return Registry::EmplaceComponent<T>(id, args...);
    }
    
    template<typename T>
    inline void DestroyComponent(){
        Registry::DestroyComponent<T>(id);
    }

    template<typename T>
    inline bool HasComponent() {
        return Registry::HasComponent<T>(id);
    }

    template<typename T>
    inline T& GetComponent() {
       return Registry::GetComponent<T>(id);
    }
    
    inline void Destroy(){
        Registry::DestroyEntity(id);
    }

    inline World* GetWorld() const {
        return Registry::GetWorld(id);
    }
    
    inline void MoveTo(World& newWorld){
        Registry::MoveEntityToWorld(id, newWorld);
    }
    
    Transform& GetTransform();
    
    // default create impl
    // define your own to hide this one
    inline void Create(){}
};
}
