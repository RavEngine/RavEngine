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
#include "ScriptComponent.hpp"
#include <future>
#include "App.hpp"
#include "PhysicsLinkSystem.hpp"

using namespace std;
using namespace RavEngine;

void RavEngine::World::Tick(float scale) {
	
    pretick(scale);
	
	//spawn entities that are pending spawn
	for (auto& e : PendingSpawn){
		Entities.insert(e);
        e->SetWorld(this);

		//start all scripts
		e->Start();
		auto coms = e->Components().GetAllComponentsOfSubclass<ScriptComponent>();
		for (auto& c : coms) {
			c->Start();
		}
        
        //make the physics system aware of this entity
        Solver->Spawn(e);
		
		//for each type of component (pair of type : list), make it available to the World
		allcomponents.AddComponentsFrom(e->Components());
	}
	PendingSpawn.clear();
	
	//Tick the game code
	midtick(scale);
	TickECS(scale);

	//process component add and removal on spawned entities
	while (!component_addremove.empty()) {
		auto& op = component_addremove.front();
		if (op.add) {
			allcomponents.AddComponentsFrom(op.store);
		}
		else {
			allcomponents.RemoveComponentsInOtherFromThis(op.store);
		}
		component_addremove.pop();
	}
	
	//destroy objects that are pending removal
	for( auto& e : PendingDestruction){
		//stop all scripts
		auto coms = e->Components().GetAllComponentsOfSubclass<ScriptComponent>();
		for (auto& c : coms) {
			c->Stop();
		}
		e->Stop();

        e->SetWorld(nullptr);
		//also remove its components
		allcomponents.RemoveComponentsInOtherFromThis(e->Components());
        
        //remove the objects from the Physics system
        Solver->Destroy(e);
		Entities.erase(e);
	}
	PendingDestruction.clear();
	
	//tick physics read
	TickSystem(plsw,scale);
    Solver->Tick(scale);
	TickSystem(plsr,scale);
    
    posttick(scale);
}


RavEngine::World::World(){
	//reserve space to reduce rehashing
	Entities.reserve(40000);
	RegisterSystem(Ref<ScriptSystem>(new ScriptSystem));
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

void RavEngine::World::TickSystem(Ref<System> system, float fpsScale){
    //get the query info
    std::list<future<void>> futures;
    auto queries = system->QueryTypes();
    for (const auto& query : queries) {
        auto temp = allcomponents.GetAllComponentsOfSubclassTypeIndex<Component>(query);
        for (auto& e : temp) {
			futures.push_back(App::threadpool.enqueue([=]{
				system->Tick(fpsScale, e.get()->getOwner());
			}));
        }
    }
    //wait for all to complete
    for (auto& f : futures) {
        f.get();
    }
}

/**
 Tick all of the objects in the world, multithreaded
 @param fpsScale the scale factor to apply to all operations based on the frame rate
 */
void RavEngine::World::TickECS(float fpsScale) {
	//bgfx::dbgTextPrintf(0, 5, 0x4f, "FPS Scale: %lf", fpsScale);
	
    //tick the systems
	for (auto& system : Systems) {
        TickSystem(system, fpsScale);
	}
}

bool RavEngine::World::InitPhysics() {
	//check if physics is already loaded (TODO: optimize)
	for (const auto& system : Systems) {
		if (dynamic_cast<PhysicsLinkSystemRead*>(system.get()) != nullptr || dynamic_cast<PhysicsLinkSystemWrite*>(system.get()) != nullptr) {
			return false;
		}
	}

	plsr = new PhysicsLinkSystemRead(Solver->scene);
	//RegisterSystem(plsr);

	plsw = new PhysicsLinkSystemWrite(Solver->scene);
	//RegisterSystem(plsw);

	return true;
}
