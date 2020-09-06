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
#include "LockLogger.hpp"
#include "ScriptComponent.hpp"
#include <uuid.h>

using namespace std;
using namespace std::chrono;
using namespace RavEngine;

const float RavEngine::World::evalNormal = 60;

void RavEngine::World::tick() {
	
	//setup framerate scaling for next frame
	auto now = clocktype::now();
	deltaTimeMicroseconds = duration_cast<timeDiff>((now-lastFrameTime));

    float deltaSeconds = deltaTimeMicroseconds.count() / 1000.0 / 1000;
    float scale = deltaSeconds * evalNormal;
    pretick(scale);
	
	//spawn entities that are pending spawn
	for (auto& e : PendingSpawn){
		Entities.insert(e);
        e->SetWorld(this);

		//start all scripts
		e->Start();
		auto& coms = e->Components().GetAllComponentsOfSubclass<ScriptComponent>();
		for (auto& c : coms) {
			c->Start();
		}
        
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
		//stop all scripts
		auto& coms = e->Components().GetAllComponentsOfSubclass<ScriptComponent>();
		for (auto& c : coms) {
			c->Stop();
		}
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


RavEngine::World::World(){
	//reserve space to reduce rehashing
	Entities.reserve(40000);
}

/**
 Spawn an entity in the world. This schedules the Entity to be added to the Entity list and its
 components to the component store at the beginning of the next tick.
 @param e the entity to spawn
 @return true if the spawn succeeded, false if it failed
 */
bool RavEngine::World::Spawn(Ref<Entity> e){
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
bool RavEngine::World::Destroy(Ref<Entity> e){
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
void RavEngine::World::tick(float fpsScale) {
	//bgfx::dbgTextPrintf(0, 5, 0x4f, "FPS Scale: %lf", fpsScale);

	//Determine the number of threads needed
	const long numthreads = std::min<long>(Entities.size(), numcpus);
	const int tasksPerThread = ceil((double)Entities.size() / numthreads);

	//bgfx::dbgTextPrintf(0, 6, 0x4f, "Threads: %d (%d per, %d total)", numthreads, tasksPerThread, Entities.size());

	for (auto& system : Systems) {
		//get the query info
		std::list<Ref<Entity>> entities;
		auto queries = system->QueryTypes();
		for (const auto& query : queries) {
			auto temp = allcomponents.GetAllComponentsOfSubclassTypeIndex<Component>(query);
			for (auto& e : temp) {
				entities.push_back(e.get()->getOwner());
			}
		}

		vector<future<void>> futures(entities.size());
		int i = 0;
		for (const auto& entity : entities) {
			//execute the system on each component
			auto ticker = [&]() {
				system->Tick(fpsScale, entity);
			};
			futures[i] = (threadpool.enqueue(ticker));
			++i;
		}

		//wait for all to complete
		for (auto& f : futures) {
			f.get();
		}
	}
}
