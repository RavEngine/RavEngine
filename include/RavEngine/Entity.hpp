#pragma once
//
//  Entity.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. 
//

#include "SharedObject.hpp"
#include "Component.hpp"
#include "Transform.hpp"
#include "WeakRef.hpp"
#include "PhysicsBodyComponent.hpp" 
#include "ComponentStore.hpp"
#include "SpinLock.hpp"
#include "DataStructures.hpp"
#include "CTTI.hpp"
/**
 This class defines an Entity for the Entity Component System.
 */
namespace RavEngine {
	class PhysicsBodyComponent;
	class World;

	class Entity : public ComponentStore<phmap::NullMutex>, public virtual_enable_shared_from_this<Entity>, public AutoCTTI {
		friend class World;
		std::unique_ptr<LinkedList<Ref<Component>>> PendingSync = std::make_unique<LinkedList<Ref<Component>>>();
	protected:
		WeakRef<World> worldptr;  //non-owning
		
		inline void AddHook(Ref<Component> c){
			c->SetOwner(shared_from_this());
			c->AddHook(shared_from_this());
		}
		
		void OnAddComponent(Ref<Component> c) override{
			//if called from a constructor, this is not valid
			//call Sync() (world invokes it automatically)
			if (!weak_from_this().expired()){
				AddHook(c);
			}
			else{
				//mark that synchronization is needed
				PendingSync->push_back(c);
			}
		}
		
		void OnRemoveComponent(Ref<Component> c) override{
			auto owner = c->GetOwner();
			owner.reset();
			c->SetOwner(owner);
			c->RemoveHook(shared_from_this());
		}


        public:
		
        inline void Sync(){
			if (PendingSync->size() != 0){
				for(auto& component : *PendingSync){
					AddHook(component);
				}
				PendingSync.reset();
			}
		}

		/**
		 Get a pointer to the world that this entity is in. May be nullptr.
		 @return the pointer to the world, or nullptr if there is no world
		 */
		inline WeakRef<World> GetWorld() const {
			return worldptr;
		}

		/**
		 Called by the world on Spawn and Destroy.
		 @param world the pointer to the current world
		 */
		inline void SetWorld(const Ref<World> world) {
			worldptr = world;
		}

		/**
		 Check if this Entity has been spawned
		 @return true if the entity is in the world, false otherwise
		 @note Does not investigate dangling pointer
		 */
        inline bool IsInWorld() const {
			return !worldptr.expired();
		}
		
		/**
		Invoked by the world when the entity is added to the World
		*/
		virtual void Start() {}

		/**
		Invoked by the world when the entity is despawned, but despawning has not happened yet. Use to preemptively clean up data.
		*/
		virtual void Stop() {}

		/**
		 Create an Entity. This constructor will add the components and their default values to the Entity.
		 */
		Entity();

		/**
		 @return a reference to the transform component, which all entities possess
		 */
		inline Ref<Transform> GetTransform() const{
			return GetComponent<RavEngine::Transform>().value();
		}

		/**
		Remove this entity from the world. If there are no more references, it will be destroyed.
		*/
		void Destroy();
        
        /**
         Convenience function to create a new anonymous Entity.
         To make something akin to a Unity Prefab or an Unreal Actor, subclass Entity
         */
        static inline Ref<Entity> New(){
            return std::make_shared<Entity>();
        }
	};
}
