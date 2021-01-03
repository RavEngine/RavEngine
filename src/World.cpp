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
#include <RmlUi/Debugger.h>
#include "GUI.hpp"
#include "InputManager.hpp"

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
	//tick physics read
	if (physicsActive){
		TickSystem(plsw,scale);
		Solver->Tick(scale);
		TickSystem(plsr,scale);
	}

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
	//cannot spawn an entity that is already in a world
	if (e->GetWorld().isNull()){
		PendingSpawn.enqueue(e);
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
	return true;
}

void RavEngine::World::TickSystem(Ref<System> system, float fpsScale){
    //get the query info
    auto queries = system->QueryTypes();
    for (const auto& query : queries) {
        auto& temp = GetAllComponentsOfTypeIndexFastPath(query);
        for (auto& e : temp) {
			tasks.emplace([=]{
				system->Tick(fpsScale, e.get()->getOwner());
			});
        }
		auto& temp2 = GetAllComponentsOfTypeIndexSubclassFastPath(query);
		for (auto& e : temp2) {
			tasks.emplace([=]{
				system->Tick(fpsScale, e.get()->getOwner());
			});
		}
    }
	//wait for all to complete
	//TODO: don't do this, instead use Taskflow's precedence-setting
	App::executor.run(tasks).wait();
	tasks.clear();
}

/**
 Tick all of the objects in the world, multithreaded
 @param fpsScale the scale factor to apply to all operations based on the frame rate
 */
void RavEngine::World::TickECS(float fpsScale) {
    //tick the systems
	for (auto& system : Systems) {
        TickSystem(system, fpsScale);
	}
}

bool RavEngine::World::InitPhysics() {
	if (physicsActive){
		return false;
	}

	plsr = new PhysicsLinkSystemRead(Solver->scene);

	plsw = new PhysicsLinkSystemWrite(Solver->scene);
	
	physicsActive = true;

	return true;
}

bool World::InitGUIDebugger(){
	if (debuggerContext.isNull()){
		debuggerContext = new Entity();
		auto ctx = debuggerContext->AddComponent<GUIComponent>(new GUIComponent());
		
		bool status = Rml::Debugger::Initialise(ctx->context);
		Spawn(debuggerContext);
		
		return true;
	}
	return false;
}

void World::BindGUIDebuggerControls(Ref<InputManager> m){
	auto ctx = debuggerContext->GetComponent<GUIComponent>();
	m->BindAnyAction<GUIComponent>(ctx);
	m->AddAxisMap("MouseX", Special::MOUSEMOVE_X);
	m->AddAxisMap("MouseY", Special::MOUSEMOVE_Y);
	
	m->BindAxis("MouseX", ctx.get(), &GUIComponent::MouseX, CID::ANY, 0);	//no deadzone
	m->BindAxis("MouseY", ctx.get(), &GUIComponent::MouseY, CID::ANY, 0);
}
