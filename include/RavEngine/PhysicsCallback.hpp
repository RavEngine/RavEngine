#pragma once
#include "WeakRef.hpp"
#include "DataStructures.hpp"
#include "ComponentHandle.hpp"
#include "Types.hpp"
#include "AddRemoveAction.hpp"
#include "Function.hpp"

/**
Multi-inherit from this interface to provide delegates for collision events
*/
namespace RavEngine {
	class PhysicsBodyComponent;
	struct ContactPairPoint;

	struct PhysicsCallback{
		/**
		Called by a PhysicsBodyComponent when it has collided with another. Override in subclasses.
		@param other the other component
		@param contactPoints the contact point data. Do not hold onto this pointer, because after this call, the pointer is invalid.
		@param numContactPoints the number of contact points. Set to 0 if wantsContactData is false.
		*/
        Function<void(PhysicsBodyComponent& other, const ContactPairPoint* contactPoints, size_t numContactPoints)> OnColliderEnter;

		/**
		Called by a PhysicsBodyComponent when it has exited collision with another. Override in subclasses.
		@param other the other component
		@param contactPoints the contact point data. Do not hold onto this pointer, because after this call, the pointer is invalid.
		@param numContactPoints the number of contact points. Set to 0 if wantsContactData is false.
		*/
        Function<void(PhysicsBodyComponent& other, const ContactPairPoint* contactPoints, size_t numContactPoints)> OnColliderExit;

		/**
		Called by a PhysicsBodyComponent when it has collided with another and the collision has persisted. Override in subclasses.
		@param other the other component
		@param contactPoints the contact point data. Do not hold onto this pointer, because after this call, the pointer is invalid.
		@param numContactPoints the number of contact points. Set to 0 if wantsContactData is false.
		*/
        Function<void(PhysicsBodyComponent& other, const ContactPairPoint* contactPoints, size_t numContactPoints)> OnColliderPersist;
		
		/**
		 Called by a PhysicsBodyComponent when it has entered another trigger . Override in subclasses. Note that triggers cannot fire events on other triggers.
		 @param other the other component
		 */
        Function<void(PhysicsBodyComponent&)>OnTriggerEnter;
		
		/**
		 Called by a PhysicsBodyComponent when it has exited another trigger . Override in subclasses. Note that triggers cannot fire events on other triggers.
		 @param other the other component
		 */
        Function<void(PhysicsBodyComponent&)>OnTriggerExit;

        void OnRegisterBody(PolymorphicComponentHandle<RavEngine::PhysicsBodyComponent> sender){
            senders.insert(sender);
        }
        void OnUnregisterBody(PolymorphicComponentHandle<RavEngine::PhysicsBodyComponent> sender){
            senders.erase(sender);
        }
        
        // invoked automatically on component destruction
        ~PhysicsCallback();

	private:
        UnorderedSet<PolymorphicComponentHandle<PhysicsBodyComponent>> senders;
        entity_t owner;
	};
}

namespace RavEngine {
    // manual specializations for this non-autogeneratable type
    template<>
    inline std::string_view type_name<PhysicsCallback>() {
        return "PhysicsCallback";
    }
}
