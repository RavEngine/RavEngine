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
    inline T& EmplaceComponent(A ... args) const{
        return Registry::EmplaceComponent<T>(id, args...);
    }
    
    template<typename T>
    inline void DestroyComponent() const{
        Registry::DestroyComponent<T>(id);
    }

    template<typename T>
    inline bool HasComponent() const{
        return Registry::HasComponent<T>(id);
    }
    
    template<typename T>
    inline bool HasComponentOfBase() const{
        return Registry::HasComponentOfBase<T>(id);
    }
    
    template<typename T>
    inline auto GetAllComponentsPolymorphic() const{
        return Registry::GetAllComponentsPolymorphic<T>(id);
    }

    template<typename T>
    inline T& GetComponent() const{
       return Registry::GetComponent<T>(id);
    }
    
    inline void Destroy(){
        Registry::DestroyEntity(id);
        id = INVALID_ENTITY;
    }
    
    inline bool IsInWorld(){
        return Registry::IsInWorld(id);
    }

    inline World* GetWorld() const {
        return Registry::GetWorld(id);
    }
    
    inline decltype(id) GetIdInWorld() const{
        return Registry::GetLocalId(id);
    }
    
    inline void MoveTo(World& newWorld) const{
        Registry::MoveEntityToWorld(id, newWorld);
    }
    
    Transform& GetTransform();
    
    // default create impl
    // define your own to hide this one
    inline void Create(){}
};
}
