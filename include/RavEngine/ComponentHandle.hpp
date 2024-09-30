#pragma once
#include "Entity.hpp"

namespace RavEngine{
    class ComponentHandleBase{
    protected:
        Entity owner;
    public:
        ComponentHandleBase(decltype(owner) owner ) : owner(owner){}
        
        inline operator bool () const{
            return IsValid();
        }
        
        inline void reset(){
            owner = Entity(INVALID_ENTITY, nullptr);
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
    
    template<typename T>
    struct ComponentHandle : public ComponentHandleBase{
        ComponentHandle(decltype(owner) owner) : ComponentHandleBase(owner){
            //assert(owner.HasComponent<T>());
        }
        ComponentHandle() : ComponentHandleBase({INVALID_ENTITY, nullptr}){}
        ComponentHandle(Entity* owner) : ComponentHandleBase(*owner){}
                
        inline T* operator->(){
            return get();
        }
        
        inline T* get(){
            //assert(EntityIsValid(owner.id));
            //assert(owner.HasComponent<T>());
            return &owner.GetComponent<T>();
        }
        
        /**
         If the type is convertible via static_cast
         */
        template<typename U>
        inline U* get_as(){
            return static_cast<U*>(get());
        }
        
        inline bool operator==(const ComponentHandle<T>& other) const{
            return owner.id == other.owner.id;
        }
    };

    template<typename Base>
    struct PolymorphicComponentHandle : ComponentHandleBase{
        ctti_t full_type_id;
        
        template<typename Full>
        PolymorphicComponentHandle(ComponentHandle<Full> fullHandle) : ComponentHandleBase(fullHandle.get_id()), full_type_id(CTTI<Full>()){
            assert(owner.HasComponentOfBase<Base>());
        }
        
        PolymorphicComponentHandle(decltype(owner) owner, decltype(full_type_id) id) : full_type_id(id),ComponentHandleBase(owner){}
        
        inline bool operator==(const PolymorphicComponentHandle<Base>& other){
            return owner.id == other.owner.id && full_type_id == other.full_type_id;
        }
        
        inline Base* get(){
            assert(EntityIsValid(owner.id));
            auto matching = owner.GetAllComponentsPolymorphic<Base>();
            for(auto& comp : matching){
                if (comp.full_id == full_type_id){
                    return comp.template Get<Base>(GetOwner().GetID());
                }
            }
            return nullptr;
        }
        
        inline Base* operator->(){
            return get();
        }
        
        template<typename T>
        inline T* get_as(){
            return static_cast<T*>(get());
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

template<typename T>
struct hash<RavEngine::PolymorphicComponentHandle<T>>{
    inline size_t operator()(const RavEngine::PolymorphicComponentHandle<T>& ch) const{
        return ch.GetOwner().id ^ ch.full_type_id;
    }
};
}
