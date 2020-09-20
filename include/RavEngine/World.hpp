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
#include "RenderEngine.hpp"
#include "ThreadPool.hpp"
#include <chrono>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include "SpinLock.hpp"

typedef std::chrono::high_resolution_clock clocktype;
typedef std::chrono::microseconds timeDiff;
typedef std::chrono::seconds deltaSeconds;
typedef std::chrono::time_point<clocktype> timePoint;

namespace RavEngine {
	class Entity;
	typedef std::unordered_set<Ref<Entity>> EntityStore;

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

		//number of cores on device
		const int numcpus = std::thread::hardware_concurrency();
		ThreadPool threadpool;

		//components data structure
		ComponentStore allcomponents;

		//Entity list
		EntityStore Entities;
		std::list<Ref<Entity>> PendingSpawn;
		std::list<Ref<Entity>> PendingDestruction;

		//Systems list (stores the loaded systems), automatically sorted in dependency order
		std::multiset<Ref<System>> Systems;

		//physics system
		Ref<PhysicsSolver> Solver = new PhysicsSolver();
		//Render engine
		Ref<RenderEngine> Renderer = new RenderEngine(this);

		//last frame time, frame delta time, framerate scale, maximum frame time
		timePoint lastFrameTime = clocktype::now();
		timeDiff deltaTimeMicroseconds;
		const timeDiff maxTimeStep = std::chrono::milliseconds((long)1000);

		template<class T>
		void RegisterSystem(Ref<T> r_instance) {
			//static_assert(std::is_base_of<System, T>::value, "Can only register systems");
			Systems.insert(r_instance);
		}

	public:
		static const float evalNormal;	//normal speed is 60 hz

		void tick();

		/**
		Resize this world's viewport to fit its window
		*/
		void resize() {
			Renderer->resize();
		}

		/**
		Called before ticking components and entities synchronously
		 @param fpsScale the scale factor calculated
		 */
		virtual void pretick(float fpsScale) {}
		virtual void tick(float);
		/**
		 Called after physics and rendering synchronously
		 @param fpsScale the scale factor calculated
		 */
		virtual void posttick(float fpsScale) {}
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