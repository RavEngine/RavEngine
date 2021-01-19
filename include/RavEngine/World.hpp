//
//  World.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#pragma once
#include "SharedObject.hpp"
#include "System.hpp"
#include "PhysicsSolver.hpp"
#include "PhysicsLinkSystem.hpp"
#include "RenderEngine.hpp"
#include "DataStructures.hpp"
#include "SystemManager.hpp"
#include "SpinLock.hpp"
#include <type_traits>
#include "ScriptSystem.hpp"
#include <taskflow/taskflow.hpp>
#include "SpinLock.hpp"

namespace RavEngine {
	class Entity;
	class InputManager;

	class World : public ComponentStore<SpinLock>, public virtual_enable_shared_from_this<World>{
	protected:
		//Entity list
		typedef locked_hashset<Ref<Entity>, SpinLock> EntityStore;
		EntityStore Entities;

		//physics system
		PhysicsSolver Solver;

		/**
		Called before ticking components and entities synchronously
		 @param fpsScale the scale factor calculated
		 */
		virtual void pretick(float fpsScale) {}
		void TickECS(float);
		/**
		 Called after physics and rendering synchronously
		 @param fpsScale the scale factor calculated
		 */
		virtual void posttick(float fpsScale) {}
        
        /**
         * Tick a System on all available threads. Blocks until all have finished.
         * @param system the System to tick
         * @param scale the frame rate scale to pass on to the System's tick method
         */
        void TickSystem(Ref<System> system, float scale);
		
		tf::Taskflow masterTasks;
		
		bool physicsActive = false;
	public:
		SystemManager systemManager;
		
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

		inline const EntityStore& getEntities() const {
			return Entities;
		}

		/**
		 Called by GameplayStatics when the final world is being deallocated
		 */
		inline void DeallocatePhysics() {
			Solver.DeallocatePhysx();
		}

		virtual ~World() {
			std::cout << "world destructor @ " << this << std::endl;
		}

		/**
		* Called when this world is made the active world for the App
		*/
		virtual void OnActivate() {}

		/**
		* Called when this world was the active world for the App but has been replaced by a different world
		*/
		virtual void OnDeactivate() {}
	};
}
