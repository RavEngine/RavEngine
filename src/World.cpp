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

static const ComponentStore<phmap::NullMutex>::entry_type emptyContainer;    // used if the query returns nothing and should be skipped

template<typename T>
static const World::SparseSet<T> staticEmptyContainer;

template<typename T>
static inline void SetEmpty(typename World::SparseSet<T>::const_iterator& begin, typename World::SparseSet<T>::const_iterator& end){
    begin = staticEmptyContainer<T>.begin();
    end = staticEmptyContainer<T>.end();
}

void RavEngine::World::Tick(float scale) {
	
    PreTick(scale);
	
	//Tick the game code
	TickECS(scale);
	

    PostTick(scale);
}


RavEngine::World::World(bool skip){
    if (!skip){
        SetupTaskGraph();
        //TODO: FIX
       // ScriptSystem s;
        //EmplaceSystem<ScriptSystem,ScriptComponent>(s);
//        systemManager.EmplaceSystem<AudioRoomSyncSystem>();
//        systemManager.EmplaceSystem<RPCSystem>();
//        systemManager.EmplaceSystem<AnimatorSystem>();
        skybox = make_shared<Skybox>();
    }
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

	//update time
	time_now = e_clock_t::now();
	
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
	
//	systemManager.RegisterSystem<PhysicsLinkSystemRead>(make_shared<PhysicsLinkSystemRead>(Solver.scene));
//	systemManager.RegisterSystem<PhysicsLinkSystemWrite>(make_shared<PhysicsLinkSystemWrite>(Solver.scene));
	
	physicsActive = true;

	return true;
}

void World::SetupTaskGraph(){
    masterTasks.name("RavEngine Master Tasks");
//	auto add_system_to_tick = [&](const SystemEntry<World>& system, ctti_t ID, SystemManager<World>::TimedSystem* ts = nullptr){
//
//		auto taskpair = system.QueryTypes(ID, iterator_map, masterTasks, this);
//		graphs[ID] = {
//			taskpair.first,
//			&system,
//			ts != nullptr
//		};
//
//		if (ts != nullptr) {
//			//conditional task - returns out-of-range if condition fails so that the task does not run
//			auto condition = masterTasks.emplace([this,ts](){
//				if (time_now - ts->last_timestamp > ts->interval){
//					ts->last_timestamp = time_now;
//						return 0;
//				}
//				return 1;
//			});
//			condition.precede(taskpair.second);
//		}
//	};
	
	//tick the always-systems
//	for (auto& s : systemManager.GetAlwaysTickSystems()) {
//		// add_system_to_tick(s.second, s.first);
//	}
//
	//tick timed systems
//	for (auto& s : systemManager.GetTimedTickSystems()){
//
//		//add_system_to_tick(s.second.system, s.first, &(s.second));
//	}
	
    //TODO: FIX (use conditional tasking here)
    setupRenderTasks();
    
    ECSTasks.name("ECS");
    ECSTaskModule = masterTasks.composed_of(ECSTasks).name("ECS");
    
    // ensure Systems run before rendering
    renderTaskModule.succeed(ECSTaskModule);
    
    // process any dispatched coroutines
    auto updateAsyncIterators = masterTasks.emplace([&]{
        async_begin = async_tasks.begin();
        async_end = async_tasks.end();
    }).name("async iterator update");
    auto doAsync = masterTasks.for_each(std::ref(async_begin), std::ref(async_end), [&](const shared_ptr<dispatched_func>& item){
        if (App::GetCurrentTime() >= item->runAtTime){
            item->func();
            ranFunctions.push_back(async_tasks.hash_for(item));
        }
    }).name("Exec Async");
    updateAsyncIterators.precede(doAsync);
    auto cleanupRanAsync = masterTasks.emplace([&]{
        // remove functions that have been run
        for(const auto hash : ranFunctions){
            async_tasks.erase_by_hash(hash);
        }
        ranFunctions.clear();
    }).name("Async cleanup");
    doAsync.precede(cleanupRanAsync);
    
	//TODO: FIX (use conditional tasking)
	if (physicsActive){
		//add the PhysX tick, must run after write but before read
		auto RunPhysics = masterTasks.emplace([this]{
			Solver.Tick(GetCurrentFPSScale());
		}).name("PhysX Tick");
        //TODO: FIX
//		RunPhysics.precede(graphs[CTTI<PhysicsLinkSystemRead>()].task).name("PhysicsLinkSystemRead");
//		RunPhysics.succeed(graphs[CTTI<PhysicsLinkSystemWrite>()].task).name("PhysicsLinkSystemWrite");
	}
    masterTasks.dump(cout);
}

void World::setupRenderTasks(){
	//render engine data collector
	//camera matrices
    renderTasks.name("Render");
    
	auto camproc = renderTasks.emplace([this](){
        if (auto allcams = GetAllComponentsOfType<CameraComponent>()){
            for (auto& cam : *allcams.value()) {
                if (cam.IsActive()) {

                    auto size = App::GetRenderEngine().GetBufferSize();
                    cam.SetTargetSize(size.width, size.height);
                    auto current = App::GetCurrentFramedata();
                    current->viewmatrix = cam.GenerateViewMatrix();
                    current->projmatrix = cam.GenerateProjectionMatrix();
                    current->cameraWorldpos = cam.GetOwner().GetTransform().GetWorldPosition();

                    break;
                }
            }
        }

	}).name("Camera data");
	
	//opaque geometry
    SetEmpty<StaticMesh>(geobegin,geoend);
    SetEmpty<Transform>(transformbegin,transformend);
    SetEmpty<SkinnedMeshComponent>(skinnedgeobegin, skinnedgeoend);
    
	auto init = renderTasks.emplace([&](){
        if(auto geometry = GetAllComponentsOfType<StaticMesh>()){
            geobegin = geometry.value()->begin();
            geoend = geometry.value()->end();
        }
        else{
            SetEmpty<StaticMesh>(geobegin,geoend);
        }
        if (auto transforms = GetAllComponentsOfType<Transform>()){
            transformbegin = transforms.value()->begin();
            transformend = transforms.value()->end();
        }
        else{
            SetEmpty<Transform>(transformbegin,transformend);
        }
        if(auto skinnedGeo = GetAllComponentsOfType<SkinnedMeshComponent>()){
            skinnedgeobegin = skinnedGeo.value()->begin();
            skinnedgeoend = skinnedGeo.value()->end();
        }
        else{
            SetEmpty<SkinnedMeshComponent>(skinnedgeobegin, skinnedgeoend);
        }
        if (auto instancedGeo = GetAllComponentsOfType<InstancedStaticMesh>()){
            instancedBegin = instancedGeo.value()->begin();
            instancedEnd = instancedGeo.value()->end();
        }
        else{
            SetEmpty<InstancedStaticMesh>(instancedBegin, instancedEnd);
        }

	}).name("Init iterators");
	
	// update matrix caches
	auto updateMatrix = [](const Transform& c) {
        c.CalculateWorldMatrix();
	};
	auto matcalc = renderTasks.for_each(std::ref(transformbegin), std::ref(transformend), updateMatrix).name("Matrix calc");
	
	//sort into the hashmap
	auto sort = renderTasks.for_each(std::ref(geobegin),std::ref(geoend),[&](const StaticMesh& e){
		auto current = App::GetCurrentFramedata();
        if (e.Enabled) {
            auto& pair = e.getTuple();
            auto mat = e.GetOwner().GetTransform().GetMatrix();
            auto& item = current->opaques[pair];
            item.AddItem(mat);
        }
	}).name("sort static");
    //TODO: FIX
	auto sortskinned = renderTasks.for_each(std::ref(skinnedgeobegin), std::ref(skinnedgeoend), [&](const SkinnedMeshComponent& e){
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
	}).name("sort skinned");
    auto sortInstanced = renderTasks.for_each(std::ref(instancedBegin), std::ref(instancedEnd), [&](const InstancedStaticMesh& m){
        auto current = App::GetCurrentFramedata();
        if (m.Enabled){
            auto& pair = m.getTuple();
            m.CalculateMatrices();
            auto& mats = m.GetAllTransforms();

            auto& item = current->opaques[pair];
            item.mtx.lock();
            item.items.insert(item.items.end(), mats.begin(),mats.end());
            item.mtx.unlock();
        }
    }).name("sort instanced");

    //TODO: FIX
	init.precede(sort/*,sortskinned*/,sortInstanced);
	matcalc.precede(sort);
//	skinnedmatcalc.precede(sortskinned);
//
	auto copydirs = renderTasks.emplace([this](){
        if (auto dirs = GetAllComponentsOfType<DirectionalLight>()){
            auto ptr = dirs.value();
            for(int i = 0; i < ptr->DenseSize(); i++){
                auto owner = Entity(ptr->GetOwner(i));
                auto rot = owner.GetTransform().Up();
                FrameData::PackedDL::tinyvec3 r{
                    static_cast<float>(rot.x),
                    static_cast<float>(rot.y),
                    static_cast<float>(rot.z)
                };
                auto current = App::GetCurrentFramedata();
                current->directionals.emplace(ptr->Get(i),r);
            }
        }
	}).name("copydirs");
	auto copyambs = renderTasks.emplace([this](){
        if(auto ambs = GetAllComponentsOfType<AmbientLight>()){
            auto ptr = ambs.value();
            for(const auto& a : *ptr){
                auto current = App::GetCurrentFramedata();
                current->ambients.emplace(a);
            }
        }

	}).name("copyambs");
	auto copyspots = renderTasks.emplace([this](){
        if(auto spots = GetAllComponentsOfType<SpotLight>()){
            auto ptr = spots.value();
            for(int i = 0; i < ptr->DenseSize(); i++){
                Entity owner(ptr->GetOwner(i));
                auto transform = owner.GetTransform().CalculateWorldMatrix();
                auto current = App::GetCurrentFramedata();
                const auto& l = ptr->Get(i);
                current->spots.emplace(l,l.CalculateMatrix(transform));
            }
        }

	}).name("copyspots");
	auto copypoints = renderTasks.emplace([this](){
        if(auto points = GetAllComponentsOfType<PointLight>()){
            auto ptr = points.value();
            for(int i = 0; i < ptr->DenseSize(); i++){
                Entity owner(ptr->GetOwner(i));
                const auto& d = ptr->Get(i);
                auto transform = owner.GetTransform().CalculateWorldMatrix();
                auto current = App::GetCurrentFramedata();
                current->points.emplace(d,d.CalculateMatrix(transform));
            }
        }

	}).name("copypoints");
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
	auto swap = renderTasks.emplace([this]{
		App::SwapCurrentFramedata();
	}).name("Swap");
	auto setup = renderTasks.emplace([this]{
		auto current = App::GetCurrentFramedata();
		current->Clear();
	}).name("Clear-setup");
	setup.precede(camproc, copydirs,copyambs,copyspots,copypoints);
	sort.precede(swap);
	sortskinned.precede(swap);
    sortInstanced.precede(swap);
	camproc.precede(sort,sortskinned);

	swap.succeed(camproc,copydirs,copyambs,copyspots,copypoints);
    
    // attatch the renderTasks module to the masterTasks
    renderTaskModule = masterTasks.composed_of(renderTasks).name("Render");
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
