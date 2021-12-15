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
#include "ComponentStore.hpp"
#include "NetworkManager.hpp"

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
        EmplacePolymorphicSystem<ScriptSystem,ScriptComponent>();
        EmplaceSystem<AnimatorSystem,AnimatorComponent>();
        CreateDependency<AnimatorSystem,ScriptSystem>();
        CreateDependency<AnimatorSystem,PhysicsLinkSystemRead>();
        CreateDependency<PhysicsLinkSystemWrite,ScriptSystem>();
        
        EmplaceSystem<AudioRoomSyncSystem, AudioRoom,Transform>();
        EmplaceSystem<RPCSystem,RPCComponent>();
        skybox = make_shared<Skybox>();
    }
}

void World::NetworkingSpawn(ctti_t id, Entity& handle){
    // are we networked, and the server?
    if (NetworkManager::IsNetworked() && NetworkManager::IsServer()){
        // is the constructed type a network object?
        if(App::networkManager.isNetworkEntity(id)){
            //add a network identity to this entity
            auto& netidcomp = handle.EmplaceComponent<NetworkIdentity>();
            
            // now send the message to spawn this on the other end
            App::networkManager.Spawn(this,id,netidcomp.GetNetworkID());
        }
    }
}

void World::NetworkingDestroy(entity_t id){
    Entity handle{id};
    // are we networked, and is this the server?
    if (NetworkManager::IsNetworked() && NetworkManager::IsServer()){
        // is this a networkobject?
        if(handle.HasComponent<NetworkIdentity>()){
            auto& netidcomp = handle.GetComponent<NetworkIdentity>();
            App::networkManager.Destroy(netidcomp.GetNetworkID());
        }
    }
}

//void World::OnRemoveComponent(Ref<Component> comp){
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
	if (isRendering){
		newFrame = true;
	}
}

bool RavEngine::World::InitPhysics() {
	if (physicsActive){
		return false;
	}
	
	physicsActive = true;

	return true;
}

void World::SetupTaskGraph(){
    masterTasks.name("RavEngine Master Tasks");
	
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
	//if (physicsActive){
		//add the PhysX tick, must run after write but before read
		auto RunPhysics = ECSTasks.emplace([this]{
			Solver.Tick(GetCurrentFPSScale());
		}).name("PhysX Tick");
    
        auto read = EmplaceSystem<PhysicsLinkSystemRead, RigidBodyDynamicComponent,Transform>(Solver.scene);
        auto write = EmplacePolymorphicSystem<PhysicsLinkSystemWrite, PhysicsBodyComponent,Transform>(Solver.scene);
        RunPhysics.precede(read.second);
		RunPhysics.succeed(write.second);
	//}
    
    // setup audio tasks
    audioTasks.name("Audio");
    
    auto audioClear = audioTasks.emplace([this]{
        App::GetCurrentAudioSnapshot()->Clear();
        //TODO: currently this selects the LAST listener, but there is no need for this
        auto fn = [](float, const auto& listener, const auto& transform){
            auto ptr = App::GetCurrentAudioSnapshot();
            ptr->listenerPos = transform.GetWorldPosition();
            ptr->listenerRot = transform.GetWorldRotation();
        };
        Filter<AudioListener,Transform>(fn);
    }).name("Clear + Listener");
    
  
    
    auto copyAudios = audioTasks.emplace([this]{
        auto fn = [this](float, auto& audioSource, auto& transform){
            App::GetCurrentAudioSnapshot()->sources.emplace_back(audioSource.GetPlayer(),transform.GetWorldPosition(),transform.GetWorldRotation());
        };
        Filter<AudioSourceComponent,Transform>(fn);
        
        // now clean up the fire-and-forget audios that have completed
        instantaneousToPlay.remove_if([](const InstantaneousAudioSource& ias){
            return ! ias.IsPlaying();
        });
        
        // now do fire-and-forget audios that need to play
        for(auto& f : instantaneousToPlay){
            App::GetCurrentAudioSnapshot()->sources.emplace_back(f.GetPlayer(),f.source_position,quaternion(0,0,0,1));
        }
    }).name("Point Audios").succeed(audioClear);
    
    auto copyAmbients = audioTasks.emplace([this]{
        if(componentMap.contains(CTTI<AmbientAudioSourceComponent>())){
            auto fn = [this](float, auto& audioSource){
                App::GetCurrentAudioSnapshot()->ambientSources.emplace_back(audioSource.GetPlayer());
            };
            Filter<AmbientAudioSourceComponent>(fn);
            
        }

        // now clean up the fire-and-forget audios that have completed
        ambientToPlay.remove_if([](const InstantaneousAmbientAudioSource& ias) {
            return !ias.IsPlaying();
        });
        
        // now do fire-and-forget audios that need to play
        for(auto& f : ambientToPlay){
            App::GetCurrentAudioSnapshot()->ambientSources.emplace_back(f.GetPlayer());
        }
        
    }).name("Ambient Audios").succeed(audioClear);
    
    auto copyRooms = audioTasks.emplace([this]{
        auto fn = [this](float, auto& room, auto& transform){
            App::GetCurrentAudioSnapshot()->rooms.emplace_back(room.data,transform.GetWorldPosition(),transform.GetWorldRotation());
        };
        Filter<AudioRoom,Transform>(fn);
        
    }).name("Rooms").succeed(audioClear);
    
    auto audioSwap = audioTasks.emplace([]{
        App::SwapCurrrentAudioSnapshot();
    }).name("Swap Current").succeed(copyAudios,copyAmbients,copyRooms);
    
    audioTaskModule = masterTasks.composed_of(audioTasks).name("Audio");
    audioTaskModule.succeed(ECSTaskModule);
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
    SetEmpty<SkinnedMeshComponent>(skinnedgeobegin, skinnedgeoend);
    
	auto init = renderTasks.emplace([&](){
        if(auto geometry = GetAllComponentsOfType<StaticMesh>()){
            geobegin = geometry.value()->begin();
            geoend = geometry.value()->end();
        }
        else{
            SetEmpty<StaticMesh>(geobegin,geoend);
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
	
	//sort into the hashmap
	auto sort = renderTasks.for_each(std::ref(geobegin),std::ref(geoend),[&](const StaticMesh& e){
		auto current = App::GetCurrentFramedata();
        if (e.Enabled) {
            auto& pair = e.getTuple();
            auto mat = e.GetOwner().GetTransform().CalculateWorldMatrix();
            auto& item = current->opaques[pair];
            item.AddItem(mat);
        }
	}).name("sort static");
	auto sortskinned = renderTasks.for_each(std::ref(skinnedgeobegin), std::ref(skinnedgeoend), [&](const SkinnedMeshComponent& m){
        if (m.Enabled) {
            auto& pair = m.getTuple();
            auto mat = m.GetOwner().GetTransform().CalculateWorldMatrix();
            auto current = App::GetCurrentFramedata();
            auto& item = current->skinnedOpaques[pair];
            item.AddItem(mat);
            // write the pose if there is one
            if (m.GetOwner().HasComponent<AnimatorComponent>()) {
                auto& animator = m.GetOwner().GetComponent<AnimatorComponent>();
                item.AddSkinningData(animator.GetSkinningMats());
            }
        }
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

	init.precede(sort,sortskinned,sortInstanced);

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
//        App::GetCurrentFramedata()->debugShapesToDraw.clear();
//        
//        FilterPolymorphic<IDebugRenderable>([&](auto scale, const auto& dbg){
//            App::GetCurrentFramedata()->debugShapesToDraw.push_back(dbg);
//        });
//	});
//#endif
	auto copyGUI = renderTasks.emplace([this]() {
        // also do the time here
        App::GetCurrentFramedata()->Time = App::GetCurrentTime();
        if(auto guis = GetAllComponentsOfType<GUIComponent>()){
            App::GetCurrentFramedata()->guisToCalculate = guis.value()->GetDense();
        }
        else{
            App::GetCurrentFramedata()->guisToCalculate.clear();
        }
	}).name("CopyGUI");

	auto swap = renderTasks.emplace([this]{
		App::SwapCurrentFramedata();
	}).name("Swap");
	auto setup = renderTasks.emplace([this]{
		auto current = App::GetCurrentFramedata();
		current->Clear();
	}).name("Clear-setup");
	setup.precede(camproc, copydirs,copyambs,copyspots,copypoints,copyGUI);
	sort.precede(swap);
	sortskinned.precede(swap);
    sortInstanced.precede(swap);
	camproc.precede(sort,sortskinned);

	swap.succeed(camproc,copydirs,copyambs,copyspots,copypoints,copyGUI);
    
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
    for(entity_t i = 0; i < localToGlobal.size(); i++){
        if (EntityIsValid(localToGlobal[i])){
            DestroyEntity(i); // destroy takes a local ID
        }
    }
}
