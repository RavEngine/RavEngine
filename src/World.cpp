//
//  World.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#include "World.hpp"
#include "GameplayStatics.hpp"
#include <iostream>
#include <algorithm>
#include "System.hpp"
#include <bgfx/bgfx.h>
#include "LockLogger.h"

using namespace std;
using namespace std::chrono;

void World::tick(){
	
	//setup framerate scaling for next frame
	auto now = clocktype::now();
	deltaTimeMicroseconds = duration_cast<timeDiff>((now-lastFrameTime));
	
	//bgfx::dbgTextPrintf(0, 4, 0x4f, "World @ %p",this);
	//bgfx::dbgTextPrintf(0, 2, 0x4f, "Frame Time: %ldms", deltaTimeMicroseconds/1000);
	//bgfx::dbgTextPrintf(0, 3, 0x4f, "FPS: %.g", std::chrono::duration<double>(1.0) / deltaTimeMicroseconds);

    float deltaSeconds = deltaTimeMicroseconds.count() / 1000.0 / 1000;
    float scale = deltaSeconds * evalNormal;
    pretick(scale);
	
	//spawn entities that are pending spawn
	for (auto& e : PendingSpawn){
		Entities.insert(e);
        e->SetWorld(this);
		e->Start();
        
        //make the render engine and the physics system aware of this entity
        Solver->Spawn(e);
        Renderer->Spawn(e);
		
		//for each type of component (pair of type : list), make it available to the World
		allcomponents.AddComponentsFrom(e->Components());
	}
	PendingSpawn.clear();
	
	//Tick the game code
	tick(scale);
	
	//destroy objects that are pending removal
	for( auto& e : PendingDestruction){
		e->Stop();
        e->SetWorld(nullptr);
		//also remove its components
		allcomponents.RemoveComponentsInOtherFromThis(e->Components());
        
        //remove the objects from the Render and Physics systems
        Renderer->Destroy(e);
        Solver->Destroy(e);
		Entities.erase(e);
	}
	PendingDestruction.clear();
	
    Solver->Tick(scale);
    Renderer->Draw();
    
    posttick(scale);
    
	lastFrameTime = now;

}


World::World(){
	//reserve space to reduce rehashing
	Entities.reserve(40000);
}

/**
 Spawn an entity in the world. This schedules the Entity to be added to the Entity list and its
 components to the component store at the beginning of the next tick.
 @param e the entity to spawn
 @return true if the spawn succeeded, false if it failed
 */
bool World::Spawn(Ref<Entity> e){
	//if the entity is not already spawned or pending spawn
	if (find(Entities.begin(), Entities.end(),e) == Entities.end() && find(PendingSpawn.begin(), PendingSpawn.end(),e) == PendingSpawn.end() ){
		mtx.lock();
		PendingSpawn.push_back(e);
		mtx.unlock();
		return true;
	}
	return false;
}

/**
 Marks an entity for destruction. This does NOT destroy it or its components immediately. At the end of the tick, entities marked with
 Destroy will be removed from the spawn list and will no longer be processed. This may or may not deallocate the Entity and its Components.
 @param e the entity to destroy
 @return true if destruction succeeded, false otherwise
 */
bool World::Destroy(Ref<Entity> e){
	//if entity is not spawned, do nothing
	if (find(Entities.begin(), Entities.end(),e) == Entities.end()){
		return false;
	}
	mtx.lock();
	PendingDestruction.push_back(e);
	mtx.unlock();
	return true;
}

/**
 Tick all of the objects in the world, multithreaded
 @param fpsScale the scale factor to apply to all operations based on the frame rate
 */
void World::tick(float fpsScale) {
	//bgfx::dbgTextPrintf(0, 5, 0x4f, "FPS Scale: %lf", fpsScale);

	//Determine the number of threads needed
	const long numthreads = std::min<long>(Entities.size(), numcpus);
	const int tasksPerThread = ceil((double)Entities.size() / numthreads);

	//bgfx::dbgTextPrintf(0, 6, 0x4f, "Threads: %d (%d per, %d total)", numthreads, tasksPerThread, Entities.size());

	/**
	 The block which processes entities on a worker thread
	 @param it the iterator marking the starting position in the Entities list to execute
	 */
	auto ticker = [&](EntityStore::iterator it) {
		//process each entity
		for (int j = 0; j < tasksPerThread && it != Entities.end(); ++j) {
			Ref<Entity> e = *it;
			//tick each system
			auto systems = e->GetSystemsOrder();
			for (auto& s : systems) {
				//get the reference to the actual system
				//try
				{
					const Ref<System>& r = Systems.at(s);
					r->Tick(fpsScale, e);
				}
				//catch (exception& err) {};			//silently catch if the system is not present
			}
			//entity's global Tick()
			e->Tick(fpsScale);
			++it;
		}
	};

	//

#define THREADPOOL
#if defined THREADPOOL
	//determine the iterators (TODO: meet in the middle on 2 threads)
	list<EntityStore::iterator> iterators;
	{
		auto it = Entities.begin();
		for (int i = 0; i < numthreads; ++i) {
			iterators.push_back(it);

			//advance the iterator to the appropriate part of the list for the next thread
			for (int k = 0; k < tasksPerThread && it != Entities.end(); ++k) {
				it = ++it;
			}
		}
	}
	
	//enqueue the tasks
	list<future<void>> tasks;
	for (const auto& begin_it : iterators) {
		tasks.push_back(threadpool.enqueue([&] {ticker(begin_it); }));
	}

	//get the results by waiting for all tasks to complete
	for (auto& future : tasks) {
		future.get();
	}
	
#else
	
	list<thread> tasks;
	for (int i = 0; i < numthreads; ++i) {
		tasks.push_back(thread(ticker,it));	//start the thread

		for (int k = 0; k < tasksPerThread && it != Entities.end(); ++k) {
			++it;
		}
		//std::advance(it, tasksPerThread)
	}
	//wait for all threads to complete
	for (auto& thread : tasks) {
		thread.join();
	}
#endif
}
