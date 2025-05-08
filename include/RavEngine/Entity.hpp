#pragma once
#include "CTTI.hpp"
#include "Layer.hpp"
#include "Types.hpp"
#include "World.hpp"

namespace RavEngine{
struct World;
struct Transform;

struct Entity : public AutoCTTI{
    World* world = nullptr;
    entity_t id = {INVALID_ENTITY};
    
    Entity(entity_t id, World* owner) : id(id), world(owner){}
    Entity(const Entity&) = default;
    Entity() = default;
    bool operator==(const Entity& other) const{
        return world == other.world && id == other.GetID();
    }
    
    template<typename T, typename ... A>
    T& EmplaceComponent(A&& ... args) const{
        return world->EmplaceComponent<T>(id, args...);
    }
    
    template<typename T>
    void DestroyComponent() const{
        world->DestroyComponent<T>(id);
    }

    template<typename T>
    bool HasComponent() const{
        return world->HasComponent<T>(id);
    }
    
    template<typename T>
    bool HasComponentOfBase() const{
        return world->HasComponentOfBase<T>(id);
    }
    
    template<typename T>
    auto GetAllComponentsPolymorphic() const{
        return world->GetAllComponentsPolymorphic<T>(id);
    }

    template<typename T>
    T& GetComponent() const{
       return world->GetComponent<T>(id);
    }
    
    entity_t GetID() const{
        return id;
    }
    
    void Destroy(){
        world->DestroyEntity(id);
        id = {INVALID_ENTITY};
    }
    
    bool IsValid() const{
        return EntityIsValid(id) && world != nullptr;
    }

    World* GetWorld() const {
        return world;
    }

    template<typename T, typename ... A>
    auto Instantiate(A&& ... args) {
        return GetWorld()->Instantiate<T>(std::forward<A>(args)...);
    }
    
    void SetEntityRenderlayer(renderlayer_t layers) const{
        world->SetEntityRenderlayer(id, layers);
    }

    void SetEntityAttributes(perobject_t attributes) {
        world->SetEntityAttributes(id, attributes);
    }

    perobject_t GetEntityAttributes() const {
        return world->GetEntityAttributes(id);
    }
    
    Transform& GetTransform();
    
    // default create impl
    // define your own to hide this one
    inline void Create(){}
};
}

namespace std{
    template<>
    struct hash<RavEngine::Entity>{
        inline size_t operator()(const RavEngine::Entity& entity) const {
            return (size_t(entity.GetID().id) << sizeof(entity_t)) | (uint32_t(uintptr_t(entity.GetWorld())));
        }
    };
}
