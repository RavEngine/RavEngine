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
#include "AudioSyncSystem.hpp"

using namespace std;
using namespace RavEngine;

void RavEngine::World::Tick(float scale) {
	
    pretick(scale);
	
	//Tick the game code
	TickECS(scale);

    posttick(scale);
}


RavEngine::World::World(){
	//reserve space to reduce rehashing
	Entities.reserve(4000);
	systemManager.RegisterSystem<ScriptSystem>(new ScriptSystem);
	systemManager.RegisterSystem<AudioSyncSystem>(new AudioSyncSystem);
}

/**
 Spawn an entity immediately
 @param e the entity to spawn
 @return true if the spawn succeeded, false if it failed
 */
bool RavEngine::World::Spawn(Ref<Entity> e){
	//cannot spawn an entity that is already in a world
	if (e->GetWorld().isNull()){
		Entities.insert(e);
		e->SetWorld(this);

		//start all scripts
		e->Start();
		auto coms = e->GetAllComponentsOfTypeSubclassFastPath<ScriptComponent>();
		for (const Ref<ScriptComponent>& c : coms) {
			c->Start();
		}

		//make the physics system aware of this entity
		Solver.Spawn(e);

		//merge the entity into the world
		Merge(*e.get());

		e->parent = this;	//set parent so that this entity synchronizes its components with this world

		//get all child entities
		auto children = e->GetAllComponentsOfTypeFastPath<ChildEntityComponent>();
		for(const Ref<ChildEntityComponent>& c : children){
			Spawn(c->get());	//spawn the child entities
		}
		return true;
	}
	return false;
}

/**
 Destroy an entity immediately
 @param e the entity to destroy
 @return true if destruction succeeded, false otherwise
 */
bool RavEngine::World::Destroy(Ref<Entity> e){
	//if entity is somehow not spawned, do nothing
	if (e->GetWorld().isNull()){
		return false;
	}
	
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
	Solver.Destroy(e);
	Entities.erase(e);
	
	//get all child entities
	auto children = e->GetAllComponentsOfTypeFastPath<ChildEntityComponent>();
	for(const Ref<ChildEntityComponent>& c : children){
		Destroy(c->get());
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

	size_t count = 0;
	for (auto& s : systemManager.GetInternalStorage()) {
		auto system = s.second;

		auto& queries = system->QueryTypes();
		count += queries.size();
	}
	phmap::flat_hash_map<ctti_t, std::array<ComponentStore::entry_type, 2>> copies(count);
	
    //tick the systems
	for (auto& s : systemManager.GetInternalStorage()) {
		auto system = s.second;
		
		auto& queries = system->QueryTypes();
		for (const auto& query : queries) {
			//add the Task to the hashmap
			
			//Avoid repeating the same query copies multiple times via hashmap
			if (!copies.contains(query)) {
				copies[query][0] = GetAllComponentsOfTypeIndexFastPath(query);
				copies[query][1] = GetAllComponentsOfTypeIndexSubclassFastPath(query);
			}

			auto func = [=](Ref<Component> e) {
				Ref<Entity> en(e->getOwner());
				if (en) {
					system->Tick(fpsScale, en);
				}
			};

			auto& l1 = copies[query][0];
			auto& l2 = copies[query][1];

			graphs[system->ID()] = {
				masterTasks.for_each(l1.begin(), l1.end(), func),
				masterTasks.for_each(l2.begin(), l2.end(),func),
				system };
		}
	}
	
	
	if (physicsActive){
		//add the PhysX tick, must run after write but before read
		auto RunPhysics = masterTasks.emplace([=]{
			Solver.Tick(fpsScale);
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
	
	systemManager.RegisterSystem<PhysicsLinkSystemRead>(new PhysicsLinkSystemRead(Solver.scene));
	systemManager.RegisterSystem<PhysicsLinkSystemWrite>(new PhysicsLinkSystemWrite(Solver.scene));
	
	physicsActive = true;

	return true;
}
