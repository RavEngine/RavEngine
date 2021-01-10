//
//  World.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#include "World.hpp"
#include <iostream>
#include <algorithm>
#include "System.hpp"
#include "ScriptComponent.hpp"
#include <future>
#include "App.hpp"
#include "PhysicsLinkSystem.hpp"
#include "GUI.hpp"
#include "InputManager.hpp"
#include "ChildEntityComponent.hpp"

using namespace std;
using namespace RavEngine;

void RavEngine::World::Tick(float scale) {
	
    pretick(scale);
	
	//spawn entities that are pending spawn
	Ref<Entity> e;
	while(PendingSpawn.try_dequeue(e)){
		Entities.insert(e);
        e->SetWorld(this);

		//start all scripts
		e->Start();
		auto coms = e->GetAllComponentsOfTypeSubclassFastPath<ScriptComponent>();
		for (const Ref<ScriptComponent>& c : coms) {
			c->Start();
		}
        
        //make the physics system aware of this entity
        Solver->Spawn(e);
		
		//merge the entity into the world
		Merge(*e.get());
		
		e->parent = this;	//set parent so that this entity synchronizes its components with this world
	}
	
	//Tick the game code
	midtick(scale);
	TickECS(scale);
	
	//destroy objects that are pending removal
	while(PendingDestruction.try_dequeue(e)){
		//stop all scripts
		auto coms = e->GetAllComponentsOfTypeSubclassFastPath<ScriptComponent>();
		for (const Ref<ScriptComponent>& c : coms) {
			c->Stop();
		}
		e->Stop();

        e->SetWorld(nullptr);
		
		//also remove its components
		Unmerge(*e.get());
		
		e->parent = nullptr;	//set parent to null so that this entity no longer synchronizes its components with this world
        
        //remove the objects from the Physics system
        Solver->Destroy(e);
		Entities.erase(e);
	}

    posttick(scale);
}


RavEngine::World::World(){
	//reserve space to reduce rehashing
	Entities.reserve(40000);
	systemManager.RegisterSystem<ScriptSystem>(new ScriptSystem);
}

/**
 Spawn an entity in the world. This schedules the Entity to be added to the Entity list and its
 components to the component store at the beginning of the next tick.
 @param e the entity to spawn
 @return true if the spawn succeeded, false if it failed
 */
bool RavEngine::World::Spawn(Ref<Entity> e){
	//cannot spawn an entity that is already in a world
	if (e->GetWorld().isNull()){
		PendingSpawn.enqueue(e);
		//get all child entities
		auto& children = e->GetAllComponentsOfTypeFastPath<ChildEntityComponent>();
		for(const Ref<ChildEntityComponent>& c : children){
			PendingSpawn.enqueue(c->get());
		}
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
	//if entity is somehow not spawned, do nothing
	if (e->GetWorld().isNull()){
		return false;
	}
	PendingDestruction.enqueue(e);
	
	//get all child entities
	auto& children = e->GetAllComponentsOfTypeFastPath<ChildEntityComponent>();
	for(const Ref<ChildEntityComponent>& c : children){
		PendingDestruction.enqueue(c->get());
	}
	return true;
}

/**
 Tick all of the objects in the world, multithreaded
 @param fpsScale the scale factor to apply to all operations based on the frame rate
 */
void RavEngine::World::TickECS(float fpsScale) {
	struct systaskpair{
		tf::Task task1, task2;
		Ref<System> system;
	};
	
	locked_hashmap<ctti_t, systaskpair, SpinLock> graphs;
	
    //tick the systems
	for (auto& s : systemManager.GetInternalStorage()) {
		auto system = s.second;
		
		auto queries = system->QueryTypes();
		for (const auto& query : queries) {
			//add the Task to the hashmap
			auto& l1 = GetAllComponentsOfTypeIndexFastPath(query);
			auto& l2 = GetAllComponentsOfTypeIndexSubclassFastPath(query);
			
			graphs[system->ID()] = {masterTasks.for_each(l1.begin(),l1.end(),[=](Ref<Component> e){
				system->Tick(fpsScale, e.get()->getOwner());
			}), masterTasks.for_each(l2.begin(),l2.end(),[=](Ref<Component> e){
					system->Tick(fpsScale, e.get()->getOwner());
				}),system};
		}
	}
	
	
	if (physicsActive){
		//add the PhysX tick, must run after write but before read
		auto RunPhysics = masterTasks.emplace([=]{
			Solver->Tick(fpsScale);
		});
		RunPhysics.precede(graphs[CTTI<PhysicsLinkSystemRead>].task1,graphs[CTTI<PhysicsLinkSystemRead>].task2);
		RunPhysics.succeed(graphs[CTTI<PhysicsLinkSystemWrite>].task1,graphs[CTTI<PhysicsLinkSystemWrite>].task2);
	}
	
	//figure out dependencies
	for(auto& graph : graphs){
		tf::Task& task1 = graph.second.task1;
		tf::Task& task2 = graph.second.task2;
		
		//call precede
		{
			auto& runbefore = graph.second.system->MustRunBefore();
			for(const auto id : runbefore){
				if (graphs.contains(id)){
					task1.precede(graphs[id].task1,graphs[id].task2);
					task2.precede(graphs[id].task1,graphs[id].task2);
				}
			}
		}
		//call succeed
		{
			auto& runafter = graph.second.system->MustRunAfter();
			for(const auto id : runafter){
				if (graphs.contains(id)){
					task1.succeed(graphs[id].task1,graphs[id].task2);
					task2.succeed(graphs[id].task1,graphs[id].task2);
				}
			}
		}
	}
	
	//execute and wait
	App::executor.run(masterTasks).wait();
	masterTasks.clear();
}

bool RavEngine::World::InitPhysics() {
	if (physicsActive){
		return false;
	}
	
	systemManager.RegisterSystem<PhysicsLinkSystemRead>(new PhysicsLinkSystemRead(Solver->scene));
	systemManager.RegisterSystem<PhysicsLinkSystemWrite>(new PhysicsLinkSystemWrite(Solver->scene));
	
	physicsActive = true;

	return true;
}
