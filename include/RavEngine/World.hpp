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
#include "Entity.hpp"
#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>


typedef std::chrono::high_resolution_clock clocktype;
typedef std::chrono::microseconds timeDiff;
typedef std::chrono::seconds deltaSeconds;
typedef std::chrono::time_point<clocktype> timePoint;


namespace RavEngine {
	typedef std::unordered_set<Ref<Entity>> EntityStore;

	class World : public SharedObject {
	protected:
		std::mutex mtx;

		//number of cores on device
		const int numcpus = std::thread::hardware_concurrency();
		ThreadPool threadpool;

		//components data structure
		ComponentStore allcomponents;

		//Entity list
		EntityStore Entities;
		std::list<Ref<Entity>> PendingSpawn;
		std::list<Ref<Entity>> PendingDestruction;

		//Systems list (stores the systems that can exist in this World)
		std::unordered_map<std::type_index, Ref<System>> Systems;

		//physics system
		Ref<PhysicsSolver> Solver = new PhysicsSolver();
		//Render engine
		Ref<RenderEngine> Renderer = new RenderEngine(this);

		//last frame time, frame delta time, framerate scale
		timePoint lastFrameTime = clocktype::now();
		timeDiff deltaTimeMicroseconds;
		const float evalNormal = 60;	//normal speed is 60 hz

		//initialize in

		template<class T>
		void RegisterSystem(Ref<T> r_instance) {
			//static_assert(std::is_convertible<ref, SystemRef>::value, "Must be a System Reference");
			Systems.insert(std::make_pair(std::type_index(typeid(T)), r_instance));
		}

	public:
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
		@returns the componentstore for this world
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
	};
}