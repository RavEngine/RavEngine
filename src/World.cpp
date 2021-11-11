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
	
    PreTick(scale);
	
	//Tick the game code
	TickECS(scale);
	

    PostTick(scale);
}


RavEngine::World::World(){
	//reserve space to reduce rehashing
	systemManager.EmplaceSystem<ScriptSystem>();
	systemManager.EmplaceSystem<AudioRoomSyncSystem>();
	systemManager.EmplaceSystem<RPCSystem>();
	systemManager.EmplaceSystem<AnimatorSystem>();
}

//void World::OnAddComponent(Ref<Component> comp){
//	//is this a script? if so, call its start
//	{
//		auto scr = dynamic_pointer_cast<ScriptComponent>(comp);
//		if (scr){
//			scr->Start();
//			return;
//		}
//	}
//	//is this a physics body? if so, call physics simulator to create it
//	{
//		auto phys = dynamic_pointer_cast<PhysicsBodyComponent>(comp);
//		auto parent = comp->GetOwner().lock();
//		if (phys && parent){
//			Solver.Spawn(parent);
//			return;
//		}
//	}
//	//is this a NetworkIdentity? if so, call Add on the NetworkManager
//	{
//		auto nid = dynamic_pointer_cast<NetworkIdentity>(comp);
//		if (nid && nid->triggerMessage) {
//            //only the server may spawn objects
//            App::networkManager.Spawn(shared_from_this(), nid);
//            return;
//		}
//	}
//}
//
//void World::OnRemoveComponent(Ref<Component> comp){
//	//is this a script? if so, call its stop
//	{
//		auto scr = dynamic_pointer_cast<ScriptComponent>(comp);
//		if (scr){
//			scr->Stop();
//			return;
//		}
//	}
//	//is this a physics body? if so, call physics simulator to stop it
//	{
//		auto phys = dynamic_pointer_cast<PhysicsBodyComponent>(comp);
//		auto parent = comp->GetOwner().lock();
//		if (phys && parent){
//			Solver.Destroy(parent);
//			return;
//		}
//	}
//	//is this a NetworkIdentity? if so, call destroy on the NetworkManager
//	{
//		auto nid = dynamic_pointer_cast<NetworkIdentity>(comp);
//		if (nid && nid->triggerMessage) {
//            //ownership is checked serverside to decide if this should be honored
//			App::networkManager.Destroy(shared_from_this(), nid);
//            return;
//		}
//	}
//}

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
    
    // process any dispatched coroutines
    auto updateAsyncIterators = masterTasks.emplace([&]{
        async_begin = async_tasks.begin();
        async_end = async_tasks.end();
    });
    auto doAsync = masterTasks.for_each(std::ref(async_begin), std::ref(async_end), [&](const shared_ptr<dispatched_func>& item){
        if (App::GetCurrentTime() >= item->runAtTime){
            item->func();
            ranFunctions.push_back(async_tasks.hash_for(item));
        }
    });
    updateAsyncIterators.precede(doAsync);
    auto cleanupRanAsync = masterTasks.emplace([&]{
        // remove functions that have been run
        for(const auto hash : ranFunctions){
            async_tasks.erase_by_hash(hash);
        }
        ranFunctions.clear();
    });
    doAsync.precede(cleanupRanAsync);
    
	
	if (physicsActive){
		//add the PhysX tick, must run after write but before read
		auto RunPhysics = masterTasks.emplace([this]{
			Solver.Tick(GetCurrentFPSScale());
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
    
    //TODO: FIX
//	auto camproc = masterTasks.emplace([this](){
//        if (auto& allcams = GetAllComponentsOfType<CameraComponent>()){
//            for (const auto& c : *allcams.value()) {
//                if (cam->IsActive()) {
//
//                    auto size = App::GetRenderEngine().GetBufferSize();
//                    cam->SetTargetSize(size.width, size.height);
//                    auto current = App::GetCurrentFramedata();
//                    current->viewmatrix = cam->GenerateViewMatrix();
//                    current->projmatrix = cam->GenerateProjectionMatrix();
//                    current->cameraWorldpos = cam->GetOwner().lock()->GetTransform()->GetWorldPosition();
//
//                    break;
//                }
//            }
//        }
//
//	});
	
	//opaque geometry
	geobegin = emptyContainer.begin();
	geoend = emptyContainer.end();
	skinnedgeobegin = emptyContainer.begin();
	skinnedgeoend = emptyContainer.end();
    
    //TODO: FIX
//	auto init = masterTasks.emplace([&](){
//        if(auto geometry = GetAllComponentsOfType<StaticMesh>()){
//            geobegin = geometry.value()->begin();
//            geoend = geometry.value()->end();
//
//        }
//        else{
//            geobegin = emptyContainer.end();
//            geoend = emptyContainer.end();
//        }
//        if(auto skinnedGeo = GetAllComponentsOfType<SkinnedMeshComponent>()){
//            skinnedgeobegin = skinnedGeo.value()->begin();
//            skinnedgeoend = skinnedGeo.value()->end();
//        }
//        else{
//            skinnedgeobegin = emptyContainer.end();
//            skinnedgeoend = emptyContainer.end();
//        }
//        if (auto instancedGeo = GetAllComponentsOfType<InstancedStaticMesh>()){
//            instancedBegin = instancedGeo.value()->begin();
//            instancedEnd = instancedGeo.value()->end();
//        }
//        else{
//            instancedBegin = emptyContainer.begin();
//            instancedEnd = emptyContainer.end();
//        }
//
//	});
	
	// update matrix caches
	auto updateMatrix = [&](const Ref<Component>& c) {
		auto owner = c->GetOwner().lock();
		if (owner) {
            owner->GetTransform().CalculateWorldMatrix();
		}
	};

	auto matcalc = masterTasks.for_each(std::ref(geobegin), std::ref(geoend), updateMatrix);
	
	auto skinnedmatcalc = masterTasks.for_each(std::ref(skinnedgeobegin),std::ref(skinnedgeoend), updateMatrix);
	
	//sort into the hashmap
    //TODO: FIX
//	auto sort = masterTasks.for_each(std::ref(geobegin),std::ref(geoend),[&](const Ref<Component>& e){
//		auto current = App::GetCurrentFramedata();
//        auto m = static_cast<StaticMesh*>(e.get());
//        auto ptr = e->GetOwner().lock();
//        if (ptr && m->Enabled) {
//            auto& pair = m->getTuple();
//            auto mat = ptr->GetTransform().GetMatrix();
//            auto& item = current->opaques[pair];
//            item.AddItem(mat);
//        }
//	});
//	auto sortskinned = masterTasks.for_each(std::ref(skinnedgeobegin), std::ref(skinnedgeoend), [&](const Ref<Component>& e){
//        auto m = static_cast<SkinnedMeshComponent*>(e.get());
//        auto ptr = e->GetOwner().lock();
//        if (ptr && m->Enabled) {
//            auto& pair = m->getTuple();
//            auto mat = ptr->GetTransform().GetMatrix();
//            auto current = App::GetCurrentFramedata();
//            auto& item = current->skinnedOpaques[pair];
//            item.AddItem(mat);
//            // write the pose if there is one
//            //TODO: FIX
////            if (auto& animator = ptr->GetComponent<AnimatorComponent>()) {
////                item.AddSkinningData(animator->GetSkinningMats());
////            }
//        }
//	});
//    auto sortInstanced = masterTasks.for_each(std::ref(instancedBegin), std::ref(instancedEnd), [&](const Ref<Component>& e){
//        auto current = App::GetCurrentFramedata();
//        auto m = static_cast<InstancedStaticMesh*>(e.get());
//        auto ptr = e->GetOwner().lock();
//        if (ptr && m->Enabled){
//            auto& pair = m->getTuple();
//            m->CalculateMatrices();
//            auto& mats = m->GetAllTransforms();
//
//            auto& item = current->opaques[pair];
//            item.mtx.lock();
//            item.items.insert(item.items.end(), mats.begin(),mats.end());
//            item.mtx.unlock();
//        }
//    });
//
    //TODO: FIX
//	init.precede(sort,sortskinned);
//	matcalc.precede(sort);
//	skinnedmatcalc.precede(sortskinned);
//
//	auto copydirs = masterTasks.emplace([this](){
//        if(auto& dirs = GetAllComponentsOfType<DirectionalLight>()){
//            for(const auto& e : dirs.value()){
//                auto owner = e->GetOwner().lock();
//                if (owner){
//                    auto d = static_cast<DirectionalLight*>(e.get());
//                    auto rot = owner->GetTransform()->Up();
//                    FrameData::PackedDL::tinyvec3 r{
//                        static_cast<float>(rot.x),
//                        static_cast<float>(rot.y),
//                        static_cast<float>(rot.z)
//                    };
//                    auto current = App::GetCurrentFramedata();
//                    current->directionals.emplace(*d,r);
//                }
//            }
//        }
//
//	});
//	auto copyambs = masterTasks.emplace([this](){
//        if(auto& ambs = GetAllComponentsOfType<AmbientLight>()){
//            for(const auto& e : ambs.value()){
//                auto d = static_cast<AmbientLight*>(e.get());
//                auto current = App::GetCurrentFramedata();
//                current->ambients.emplace(*d);
//            }
//        }
//
//	});
//	auto copyspots = masterTasks.emplace([this](){
//        if(auto& spots = GetAllComponentsOfType<SpotLight>()){
//            for(const auto& e : spots.value()){
//                auto d = static_cast<SpotLight*>(e.get());
//                auto ptr = e->GetOwner().lock();
//                if (ptr){
//                    auto transform = ptr->GetTransform()->CalculateWorldMatrix();
//                    auto current = App::GetCurrentFramedata();
//                    current->spots.emplace(*d,d->CalculateMatrix(transform));
//                }
//            }
//        }
//
//	});
//	auto copypoints = masterTasks.emplace([this](){
//        if(auto& points = GetAllComponentsOfType<PointLight>()){
//            for(const auto& e : points.value()){
//                auto d = static_cast<PointLight*>(e.get());
//                auto ptr = e->GetOwner().lock();
//                if (ptr){
//                    auto transform = ptr->GetTransform()->CalculateWorldMatrix();
//                    auto current = App::GetCurrentFramedata();
//                    current->points.emplace(*d,d->CalculateMatrix(transform));
//                }
//            }
//        }
//
//	});
//
//#ifdef _DEBUG
//	// copy debug shapes
//	auto copyDebug = masterTasks.emplace([this]() {
//        if(auto& dbg = GetAllComponentsOfType<IDebugRenderable>()){
//            App::GetCurrentFramedata()->debugShapesToDraw = dbg.value().get_underlying();
//
//        }
//        else{
//            App::GetCurrentFramedata()->debugShapesToDraw.clear();
//        }
//	});
//#endif
//	auto copyGUI = masterTasks.emplace([this]() {
//        // also do the time here
//        App::GetCurrentFramedata()->Time = App::GetCurrentTime();
//        if(auto& guis = GetAllComponentsOfType<GUIComponent>()){
//            App::GetCurrentFramedata()->guisToCalculate = guis.value().get_underlying();
//        }
//        else{
//            App::GetCurrentFramedata()->guisToCalculate.clear();
//        }
//	});
//
//	auto swap = masterTasks.emplace([this]{
//		App::SwapCurrentFramedata();
//	});
//	auto setup = masterTasks.emplace([this]{
//		auto current = App::GetCurrentFramedata();
//		current->Clear();
//	});
//	setup.precede(camproc,copydirs,copyambs,copyspots,copypoints);
//	sort.precede(swap);
//	sortskinned.precede(swap);
//    sortInstanced.precede(swap);
//	camproc.precede(sort,sortskinned);
//
//	//ensure user code completes before framedata population
//	for(auto& g : graphs){
//		if (!g.second.isTimed){
//			g.second.task.precede(camproc,copydirs,copyambs,copyspots,copypoints,matcalc,skinnedmatcalc,sort,sortskinned, copyGUI, sortInstanced
//#ifdef _DEBUG
//				, copyDebug
//#endif
//			);
//		}
//	}
//
//	swap.succeed(camproc,copydirs,copyambs,copyspots,copypoints);
}

void World::DispatchAsync(const Function<void ()>& func, double delaySeconds){
    auto time = App::GetCurrentTime();
    App::DispatchMainThread([=]{
        async_tasks.insert(make_shared<dispatched_func>(time + delaySeconds,func));
    });
}

entity_t World::CreateEntity(){
    entity_t id;
    if (available.size() > 0){
        id = available.front();
        available.pop();
    }
    else{
        id = localToGlobal.size();
        localToGlobal.push_back(INVALID_ENTITY);
    }
    localToGlobal[id] = Registry::CreateEntity(this, id);
    return localToGlobal[id];
}

World::~World() {
    for (const auto& e : localToGlobal) {
        if (EntityIsValid(e)) {
            Registry::ReleaseEntity(e);
        }
    }
}
