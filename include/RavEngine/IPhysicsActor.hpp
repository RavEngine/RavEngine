#pragma once
#include "WeakRef.hpp"
#include "DataStructures.hpp"
#include "SharedObject.hpp"
#include "ComponentHandle.hpp"
#include "Types.hpp"
#include "AddRemoveAction.hpp"
#include <uuids.h>

/**
Multi-inherit from this interface to provide delegates for collision events
*/
namespace RavEngine {
	class PhysicsBodyComponent;
	struct ContactPairPoint;
    struct IPhysicsActor;

    struct Receiver{
        friend class IPhysicsActor;
        
        const entity_t owner;
        //const ctti_t type;
        Function<IPhysicsActor*(void)> fn;
        const uuids::uuid ipa_id;
        
        template<typename T>
        Receiver(ComponentHandle<T> handle) : owner(handle.GetOwner().id),/* type(CTTI<T>()),*/ ipa_id(handle->ipa_id),  fn([handle]() mutable{
            return handle.template get_as<IPhysicsActor>();
        }){}
        
        inline bool operator==(const Receiver& other) const{
            return /*type == other.type &&*/ other.owner == owner && other.ipa_id == ipa_id;
        }
        
        inline IPhysicsActor* operator->() const{
            return fn();
        }
    private:
        /**
         for internal construction only
         */
        Receiver(IPhysicsActor* actor);
    };

	struct IPhysicsActor : public virtual_enable_shared_from_this<IPhysicsActor>{
        friend class Receiver;
        IPhysicsActor(entity_t owner) : owner(owner){}
		/**
		Called by a PhysicsBodyComponent when it has collided with another. Override in subclasses.
		@param other the other component
		@param contactPoints the contact point data. Do not hold onto this pointer, because after this call, the pointer is invalid.
		@param numContactPoints the number of contact points. Set to 0 if wantsContactData is false.
		*/
		virtual void OnColliderEnter(ComponentHandle<RavEngine::PhysicsBodyComponent> other, const ContactPairPoint* contactPoints, size_t numContactPoints) {}

		/**
		Called by a PhysicsBodyComponent when it has exited collision with another. Override in subclasses.
		@param other the other component
		@param contactPoints the contact point data. Do not hold onto this pointer, because after this call, the pointer is invalid.
		@param numContactPoints the number of contact points. Set to 0 if wantsContactData is false.
		*/
		virtual void OnColliderExit(ComponentHandle<RavEngine::PhysicsBodyComponent> other, const ContactPairPoint* contactPoints, size_t numContactPoints) {}

		/**
		Called by a PhysicsBodyComponent when it has collided with another and the collision has persisted. Override in subclasses.
		@param other the other component
		@param contactPoints the contact point data. Do not hold onto this pointer, because after this call, the pointer is invalid.
		@param numContactPoints the number of contact points. Set to 0 if wantsContactData is false.
		*/
		virtual void OnColliderPersist(ComponentHandle<RavEngine::PhysicsBodyComponent> other, const ContactPairPoint* contactPoints, size_t numContactPoints) {}
		
		/**
		 Called by a PhysicsBodyComponent when it has entered another trigger . Override in subclasses. Note that triggers cannot fire events on other triggers.
		 @param other the other component
		 */
		virtual void OnTriggerEnter(ComponentHandle<RavEngine::PhysicsBodyComponent>){}
		
		/**
		 Called by a PhysicsBodyComponent when it has exited another trigger . Override in subclasses. Note that triggers cannot fire events on other triggers.
		 @param other the other component
		 */
		virtual void OnTriggerExit(ComponentHandle<RavEngine::PhysicsBodyComponent>){}

        void OnRegisterBody(ComponentHandle<RavEngine::PhysicsBodyComponent> sender){
            senders.insert(sender);
        }
        void OnUnregisterBody(ComponentHandle<RavEngine::PhysicsBodyComponent> sender){
            senders.erase(sender);
        }
        
        // invoked automatically on component destruction
        void OnDestroy();
    
	private:
        uuids::uuid ipa_id = uuids::uuid::create();
        UnorderedSet<ComponentHandle<PhysicsBodyComponent>> senders;
        entity_t owner;
	};
}

namespace std{
    template<>
    struct hash<RavEngine::Receiver>{
        inline size_t operator()(const RavEngine::Receiver& obj){
            return obj.owner; // TODO: make hash better by including uuid?
        }
    };
}
