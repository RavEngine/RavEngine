#pragma once
#include "Entity.hpp"

namespace RavEngine{
    template<typename T>
    class ComponentHandle{
        Entity owner;
    public:
        ComponentHandle(decltype(owner) owner) : owner(owner){
            assert(owner.HasComponent<T>());
        }
        ComponentHandle() : owner(INVALID_ENTITY){}
        
        inline T* operator->(){
            return get();
        }
        
        inline T* get(){
            assert(EntityIsValid(owner.id));
            assert(owner.HasComponent<T>());
            return &owner.GetComponent<T>();
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
