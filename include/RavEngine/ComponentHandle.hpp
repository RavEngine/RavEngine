#pragma once
#include "Entity.hpp"

namespace RavEngine{
    class ComponentHandleBase{
    protected:
        Entity owner;
    public:
        ComponentHandleBase(decltype(owner) owner ) : owner(owner){}
    };
    
    template<typename T>
    struct ComponentHandle : public ComponentHandleBase{
        ComponentHandle(decltype(owner) owner) : ComponentHandleBase(owner){
            assert(owner.HasComponent<T>());
        }
        ComponentHandle() : ComponentHandleBase(INVALID_ENTITY){}
        
        ComponentHandle(Entity* owner) : ComponentHandleBase(owner->id){}
        ComponentHandle(entity_t ID) : ComponentHandleBase(ID){}
        
        inline T* operator->(){
            return get();
        }
        
        inline T* get(){
            assert(EntityIsValid(owner.id));
            assert(owner.HasComponent<T>());
            return &owner.GetComponent<T>();
        }
        
        /**
         If the type is convertible via static_cast
         */
        template<typename U>
        inline U* get_as(){
            return static_cast<U*>(get());
        }
        
        inline operator bool () const{
            return IsValid();
        }
        
        inline void reset(){
            owner = Entity(INVALID_ENTITY);
        }
        
        inline bool IsValid() const{
            return EntityIsValid(owner.id);
        }
        
        inline decltype(owner) GetOwner() const{
            return owner;
        }
        
        inline entity_t get_id() const{
            return GetOwner().id;
        }
        
        inline bool operator==(ComponentHandle<T>& other) const{
            return owner.id == other.owner.id;
        }
    };

    template<typename T>
    struct PolymorphicComponentHandle : ComponentHandleBase{
        PolymorphicComponentHandle(decltype(owner) owner) : ComponentHandleBase(owner){
            assert(owner.HasComponentOfBase<T>());
        }
        PolymorphicComponentHandle() : ComponentHandleBase(INVALID_ENTITY){}
        
        PolymorphicComponentHandle(Entity* owner) : ComponentHandleBase(owner->id){}
        PolymorphicComponentHandle(entity_t ID) : ComponentHandleBase(ID){}
        
        
    };
}

namespace std{
    template<typename T>
    struct hash<RavEngine::ComponentHandle<T>>{
        inline size_t operator()(const RavEngine::ComponentHandle<T>& ch) const{
            return ch.GetOwner().id;
        }
    };
}
