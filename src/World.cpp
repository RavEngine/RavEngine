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
#include "App.hpp"
#include "PhysicsLinkSystem.hpp"
#include "GUI.hpp"
#include "InputManager.hpp"
#include "ChildEntityComponent.hpp"
#include "AudioRoomSyncSystem.hpp"
#include "CameraComponent.hpp"
#include "StaticMesh.hpp"
#include "BuiltinMaterials.hpp"
#include "NetworkIdentity.hpp"
#include "RPCSystem.hpp"
#include "AnimatorSystem.hpp"
#include "SkinnedMeshComponent.hpp"

using namespace std;
using namespace RavEngine;

void RavEngine::World::Tick(float scale) {
	
    pretick(scale);
	
	//Tick the game code
	TickECS(scale);
	
	//apply component changes
	SyncOp op;
	while (toSync.try_dequeue(op)){
		if (op.add){
			ComponentStore::CTTI_Add(op.c, op.id);
		}
		else{
			ComponentStore::CTTI_Remove(op.c, op.id);
		}
	}

    posttick(scale);
	Destroy_pending();
}


RavEngine::World::World(){
	//reserve space to reduce rehashing
	systemManager.EmplaceSystem<ScriptSystem>();
	systemManager.EmplaceSystem<AudioRoomSyncSystem>();
	systemManager.EmplaceSystem<RPCSystem>();
	systemManager.EmplaceSystem<AnimatorSystem>();
}

/**
 Spawn an entity immediately
 @param e the entity to spawn
 @return true if the spawn succeeded, false if it failed
 */
bool RavEngine::World::Spawn(Ref<Entity> e){
	//cannot spawn an entity that is already in a world
	if (e->GetWorld().expired()){
		Entities.insert(e);
		e->Sync();	//ensure all components have their owner backpointers up-to-date
		e->SetWorld(shared_from_this());

		//start entity
		e->Start();

		//merge the entity into the world
		Merge(*e.get());

		e->parent = shared_from_this();	//set parent so that this entity synchronizes its components with this world

		//get all child entities
		auto children = e->GetAllComponentsOfType<ChildEntityComponent>();
		for(const auto c : children){
			Spawn(std::static_pointer_cast<ChildEntityComponent>(c)->get());	//spawn the child entities
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
	if (e->GetWorld().expired()){
		return false;
	}
	e->Stop();
	to_destroy.insert(e);

	//get all child entities
	auto children = e->GetAllComponentsOfType<ChildEntityComponent>();
	for (const auto c : children) {
		Destroy(std::static_pointer_cast<ChildEntityComponent>(c)->get());
	}
	
	return true;
}

void World::OnAddComponent(Ref<Component> comp){
	//is this a script? if so, call its start
	{
		auto scr = dynamic_pointer_cast<ScriptComponent>(comp);
		if (scr){
			scr->Start();
			return;
		}
	}
	//is this a physics body? if so, call physics simulator to create it
	{
		auto phys = dynamic_pointer_cast<PhysicsBodyComponent>(comp);
		auto parent = comp->getOwner().lock();
		if (phys && parent){
			Solver.Spawn(parent);
			return;
		}
	}
	//is this a NetworkIdentity? if so, call Add on the NetworkManager
	{
		auto nid = dynamic_pointer_cast<NetworkIdentity>(comp);
		if (nid && nid->triggerMessage) {
            //only the server may spawn objects
            App::networkManager.Spawn(shared_from_this(), nid);
            return;
		}
	}
}

void World::OnRemoveComponent(Ref<Component> comp){
	//is this a script? if so, call its stop
	{
		auto scr = dynamic_pointer_cast<ScriptComponent>(comp);
		if (scr){
			scr->Stop();
			return;
		}
	}
	//is this a physics body? if so, call physics simulator to stop it
	{
		auto phys = dynamic_pointer_cast<PhysicsBodyComponent>(comp);
		auto parent = comp->getOwner().lock();
		if (phys && parent){
			Solver.Destroy(parent);
			return;
		}
	}
	//is this a NetworkIdentity? if so, call destroy on the NetworkManager
	{
		auto nid = dynamic_pointer_cast<NetworkIdentity>(comp);
		if (nid && nid->triggerMessage) {
            //ownership is checked serverside to decide if this should be honored
			App::networkManager.Destroy(shared_from_this(), nid);
            return;
		}
	}
}

void RavEngine::World::Destroy_pending()
{
	for (const auto& e : to_destroy) {
		e->SetWorld(nullptr);

		//also remove its components
		Unmerge(*e.get());

		e->parent.reset();	//set parent to null so that this entity no longer synchronizes its components with this world

		Entities.erase(e);
	}
	to_destroy.clear();
}

/**
 Tick all of the objects in the world, multithreaded
 @param fpsScale the scale factor to apply to all operations based on the frame rate
 */
void RavEngine::World::TickECS(float fpsScale) {
	currentFPSScale = fpsScale;
	if (systemManager.graphNeedsRebuild){
		RebuildTaskGraph();
	}
	
	//update time
	time_now = clock_t::now();
	
	//execute and wait
	App::executor.run(masterTasks).wait();
	//masterTasks.dump(std::cout);
	if (isRendering){
		newFrame = true;
	}
}

bool RavEngine::World::InitPhysics() {
	if (physicsActive){
		return false;
	}
	
	systemManager.RegisterSystem<PhysicsLinkSystemRead>(make_shared<PhysicsLinkSystemRead>(Solver.scene));
	systemManager.RegisterSystem<PhysicsLinkSystemWrite>(make_shared<PhysicsLinkSystemWrite>(Solver.scene));
	
	physicsActive = true;

	return true;
}

void World::RebuildTaskGraph(){
	masterTasks.clear();
	
	auto add_system_to_tick = [&](const SystemEntry<World>& system, ctti_t ID, SystemManager<World>::TimedSystem* ts = nullptr){

		auto taskpair = system.QueryTypes(ID, iterator_map, masterTasks, this);
		graphs[ID] = {
			taskpair.first,
			&system,
			ts != nullptr
		};

		if (ts != nullptr) {
			//conditional task - returns out-of-range if condition fails so that the task does not run
			auto condition = masterTasks.emplace([this,ts](){
				if (time_now - ts->last_timestamp > ts->interval){
					ts->last_timestamp = time_now;
						return 0;
				}
				return 1;
			});
			condition.precede(taskpair.second);
		}
	};
	
	//tick the always-systems
	for (auto& s : systemManager.GetAlwaysTickSystems()) {
		add_system_to_tick(s.second, s.first);
	}
	
	//tick timed systems
	for (auto& s : systemManager.GetTimedTickSystems()){
		
		add_system_to_tick(s.second.system, s.first, &(s.second));
	}
	
	if (isRendering)
	{
		FillFramedata();
	}
	
	if (physicsActive){
		//add the PhysX tick, must run after write but before read
		auto RunPhysics = masterTasks.emplace([this]{
			Solver.Tick(getCurrentFPSScale());
		});
		RunPhysics.precede(graphs[CTTI<PhysicsLinkSystemRead>()].task);
		RunPhysics.succeed(graphs[CTTI<PhysicsLinkSystemWrite>()].task);
	}
	
	//figure out dependencies
	for(auto& graph : graphs){
		tf::Task& task = graph.second.task;
		
		//call precede
		{
			auto& runbefore = graph.second.system->MustRunBefore();
			for(const auto id : runbefore){
				if (graphs.find(id) != graphs.end()){
					task.precede(graphs[id].task);
				}
			}
		}
		//call succeed
		{
			auto& runafter = graph.second.system->MustRunAfter();
			for(const auto id : runafter){
				if (graphs.find(id) != graphs.end()){
					task.succeed(graphs[id].task);
				}
			}
		}
	}
	systemManager.graphNeedsRebuild = false;
}

void World::FillFramedata(){
	//render engine data collector
	//camera matrices
	auto camproc = masterTasks.emplace([this](){
		auto& allcams = GetAllComponentsOfType<CameraComponent>();
		for (const auto& c : allcams) {
			auto cam = std::static_pointer_cast<CameraComponent>(c);
			if (cam->isActive()) {
				
				auto size = App::Renderer->GetBufferSize();
				cam->SetTargetSize(size.width, size.height);
				auto current = App::GetCurrentFramedata();
				current->viewmatrix = cam->GenerateViewMatrix();
				current->projmatrix = cam->GenerateProjectionMatrix();
				
				break;
			}
		}
	});
	
	//opaque geometry
	geobegin = GetAllComponentsOfType<StaticMesh>().begin();
	geoend = GetAllComponentsOfType<StaticMesh>().end();
	skinnedgeobegin = GetAllComponentsOfType<SkinnedMeshComponent>().begin();
	skinnedgeoend = GetAllComponentsOfType<SkinnedMeshComponent>().end();
	auto init = masterTasks.emplace([&](){
		auto& geometry = GetAllComponentsOfType<StaticMesh>();
		geobegin = geometry.begin();
		geoend = geometry.end();
		auto& skinnedGeo = GetAllComponentsOfType<SkinnedMeshComponent>();
		skinnedgeobegin = skinnedGeo.begin();
		skinnedgeoend = skinnedGeo.end();
	});
	
	// update matrix caches
	auto matcalc = masterTasks.for_each(std::ref(geobegin), std::ref(geoend), [&](const Ref<Component>& c){
		auto owner = c->getOwner().lock();
		if (owner){
			owner->transform()->CalculateWorldMatrix();
		}
	});
	
	auto skinnedmatcalc = masterTasks.for_each(std::ref(skinnedgeobegin),std::ref(skinnedgeoend), [&](const Ref<Component>& c){
		auto owner = c->getOwner().lock();
		if (owner){
			owner->transform()->CalculateWorldMatrix();
		}
	});
	
	//sort into the hashmap
	auto sort = masterTasks.emplace([&]{
		auto current = App::GetCurrentFramedata();
		for (auto it = geobegin; it != geoend; ++it) {
			const auto& e = *it;
			auto m = static_cast<StaticMesh*>(e.get());
			auto ptr = e->getOwner().lock();
			if (ptr && m->Enabled) {
				auto& pair = m->getTuple();
				auto mat = ptr->transform()->GetMatrix();
				auto& item = current->opaques[pair];
				item.items.push_back(mat);
			}
		}
	});
	auto sortskinned = masterTasks.emplace([&]{

		for (auto it = skinnedgeobegin; it != skinnedgeoend; ++it) {
			const auto& e = *it;
			auto m = static_cast<SkinnedMeshComponent*>(e.get());
			auto ptr = e->getOwner().lock();
			if (ptr && m->Enabled) {
				auto& pair = m->getTuple();
				auto mat = ptr->transform()->GetMatrix();
				auto current = App::GetCurrentFramedata();
				auto& item = current->skinnedOpaques[pair];
				item.items.push_back(mat);
				// write the pose if there is one
				if (auto animator = ptr->GetComponent<AnimatorComponent>()) {
					item.skinningdata.push_back(animator.value()->GetSkinningMats());
				}
			}
		}
		
	});
	init.precede(sort,sortskinned);
	matcalc.precede(sort);
	skinnedmatcalc.precede(sortskinned);
	
	auto copydirs = masterTasks.emplace([this](){
		auto& dirs = GetAllComponentsOfType<DirectionalLight>();
		for(const auto& e : dirs){
			auto owner = e->getOwner().lock();
			if (owner){
				auto d = static_cast<DirectionalLight*>(e.get());
				auto rot = owner->transform()->Up();
				FrameData::PackedDL::tinyvec3 r{
					static_cast<float>(rot.x),
					static_cast<float>(rot.y),
					static_cast<float>(rot.z)
				};
				auto current = App::GetCurrentFramedata();
				current->directionals.emplace(*d,r);
			}
		}
	});
	auto copyambs = masterTasks.emplace([this](){
		auto& ambs = GetAllComponentsOfType<AmbientLight>();
		for(const auto& e : ambs){
			auto d = static_cast<AmbientLight*>(e.get());
			auto current = App::GetCurrentFramedata();
			current->ambients.emplace(*d);
		}
	});
	auto copyspots = masterTasks.emplace([this](){
		auto& spots = GetAllComponentsOfType<SpotLight>();
		for(const auto& e : spots){
			auto d = static_cast<SpotLight*>(e.get());
			auto ptr = e->getOwner().lock();
			if (ptr){
				auto transform = ptr->transform()->CalculateWorldMatrix();
				auto current = App::GetCurrentFramedata();
				current->spots.emplace(*d,d->CalculateMatrix(transform));
			}
		}
	});
	auto copypoints = masterTasks.emplace([this](){
		auto& points = GetAllComponentsOfType<PointLight>();
		for(const auto& e : points){
			auto d = static_cast<PointLight*>(e.get());
			auto ptr = e->getOwner().lock();
			if (ptr){
				auto transform = ptr->transform()->CalculateWorldMatrix();
				auto current = App::GetCurrentFramedata();
				current->points.emplace(*d,d->CalculateMatrix(transform));
			}
		}
	});
	
	
	auto swap = masterTasks.emplace([this]{
		App::SwapCurrentFramedata();
	});
	auto setup = masterTasks.emplace([this]{
		auto current = App::GetCurrentFramedata();
		current->Clear();
	});
	setup.precede(camproc,copydirs,copyambs,copyspots,copypoints);
	sort.precede(swap);
	sortskinned.precede(swap);
	camproc.precede(sort,sortskinned);
	
	//ensure user code completes before framedata population
	for(auto& g : graphs){
		if (!g.second.isTimed){
			g.second.task.precede(camproc,copydirs,copyambs,copyspots,copypoints);
		}
	}
	
	swap.succeed(camproc,copydirs,copyambs,copyspots,copypoints);
}
