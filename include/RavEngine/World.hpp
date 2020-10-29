//
//  World.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once
#include "SharedObject.hpp"
#include "System.hpp"
#include "PhysicsSolver.hpp"
#include "PhysicsLinkSystem.hpp"
#include "RenderEngine.hpp"
#include <phmap.h>
#include <queue>
#include <set>
#include "SpinLock.hpp"
#include <type_traits>
#include "ScriptSystem.hpp"

namespace RavEngine {
	class Entity;
	typedef phmap::parallel_flat_hash_set<Ref<Entity>> EntityStore;

	class World : public SharedObject {
	protected:
		SpinLock mtx;
		SpinLock component_op_mtx;

		//for adding and removing components on spawned entities
		struct component_operation {
			bool add = true;
			ComponentStore store;

			/**
			Create an add / remove operation
			@param add true if the contents of this store should be merged, or used to delete
			@param s the component store to process
			*/
			component_operation(bool add, const ComponentStore& s) : add(add), store(s) {}
		};
		std::queue<component_operation> component_addremove;

		//components data structure
		ComponentStore allcomponents;

		//Entity list
		EntityStore Entities;
		plf::list<Ref<Entity>> PendingSpawn;
		plf::list<Ref<Entity>> PendingDestruction;

		//Systems list (stores the loaded systems), automatically sorted in dependency order
		std::multiset<Ref<System>> Systems;

		//physics system
		Ref<PhysicsSolver> Solver = new PhysicsSolver();
		Ref<PhysicsLinkSystemRead> plsr;
		Ref<PhysicsLinkSystemWrite> plsw;
        
		template<class T>
		void RegisterSystem(Ref<T> r_instance) {
			//static_assert(std::is_base_of<System, T>::value, "Can only register systems");
			Systems.insert(r_instance);
		}

		/**
		Called before ticking components and entities synchronously
		 @param fpsScale the scale factor calculated
		 */
		virtual void pretick(float fpsScale) {}
		virtual void midtick(float fpsScale) {}
		void TickECS(float);
		/**
		 Called after physics and rendering synchronously
		 @param fpsScale the scale factor calculated
		 */
		virtual void posttick(float fpsScale) {}
        
        /**
         * Tick a System on all available threads. For internal use only. Blocks until all have finished.
         * @param system the System to tick
         * @param scale the frame rate scale to pass on to the System's tick method
         */
        void TickSystem(Ref<System> system, float scale);

	public:
		/**
		* Initializes the physics-related Systems.
		* @return true if the systems were loaded, false if they were not loaded because they are already loaded
		*/
		bool InitPhysics();

		/**
		* Evaluate the world given a scale factor. One tick = 1/App::EvalNormal
		* @param the tick fraction to evaluate
		* @note the GameplayStatics CurrentWorld is ticked automatically in the App
		*/
		void Tick(float);

		World();

		//spawn function (takes an Entity)
		bool Spawn(Ref<Entity>);

		bool Destroy(Ref<Entity>);

		const EntityStore& getEntities() const {
			return Entities;
		}

		/**
		@returns the componentstore for this world. Do not use in scripts!
		*/
		const ComponentStore& Components() {
			return allcomponents;
		}

		/**
		 Called by GameplayStatics when the final world is being deallocated
		 */
		void DeallocatePhysics() {
			Solver->DeallocatePhysx();
		}

		virtual ~World() {
			std::cout << "world destructor @ " << this << std::endl;
		}

		void AddComponentsSpawnedEntity(const ComponentStore& store) {
			component_op_mtx.lock();
			component_addremove.emplace(true,store);
			component_op_mtx.unlock();
		}

		void RemoveComponentsSpawnedEntity(const ComponentStore& store) {
			component_op_mtx.lock();
			component_addremove.emplace(false,store);
			component_op_mtx.unlock();
		}
	};
}
