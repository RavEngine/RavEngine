//
//  Entity.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. 
//

#pragma once
#include "SharedObject.hpp"
#include "Component.hpp"
#include "Transform.hpp"
#include "WeakRef.hpp"
#include "PhysicsBodyComponent.hpp" 
#include "ComponentStore.hpp"
#include "SpinLock.hpp"

/**
 This class defines an Entity for the Entity Component System.
 */
namespace RavEngine {
	class PhysicsBodyComponent;
	class World;

	class Entity : public ComponentStore<SpinLock>, public std::enable_shared_from_this<Entity> {
		friend class World;
	protected:
		WeakRef<World> worldptr;  //non-owning
		
		void OnAddComponent(Ref<Component> c) override{
			c->SetOwner(weak_from_this());
			c->AddHook(weak_from_this());
		}
		
		void OnRemoveComponent(Ref<Component> c) override{
			c->SetOwner(weak_from_this());
			c->RemoveHook(weak_from_this());
		}


	public:

		//required virtual destructor for SharedObject
		virtual ~Entity();

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
		inline void SetWorld(const WeakRef<World>& world) {
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
		inline Ref<Transform> transform(){
			return GetComponent<Transform>();
		}

		/**
		Remove this entity from the world. If there are no more references, it will be destroyed.
		*/
		void Destroy();

	protected:
		void SyncAdds();
		void SyncRemovals();
	};
}
