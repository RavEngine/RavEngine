//
//  World.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#include "World.hpp"
#include <iostream>
#include <algorithm>
#include "ScriptComponent.hpp"
#include "App.hpp"
#include "PhysicsLinkSystem.hpp"
#include "GUI.hpp"
#include "InputManager.hpp"
#include "AudioRoomSyncSystem.hpp"
#include "CameraComponent.hpp"
#include "StaticMesh.hpp"
#include "BuiltinMaterials.hpp"
#include "NetworkIdentity.hpp"
#include "RPCSystem.hpp"
#include "AnimatorSystem.hpp"
#include "SkinnedMeshComponent.hpp"
#include "NetworkManager.hpp"
#include "Constraint.hpp"
#include <physfs.h>
#include "ScriptSystem.hpp"
#include "RenderEngine.hpp"
#include "Skybox.hpp"
#include "PhysicsSolver.hpp"
#if !RVE_SERVER
    #include "VRAMSparseSet.hpp"
#else
    #include "Transform.hpp"
#endif
#include "PhysicsBodyComponent.hpp"

using namespace std;
using namespace RavEngine;

template<typename T>
static const World::EntitySparseSet<T> staticEmptyContainer;

template<typename T>
static inline void SetEmpty(typename World::EntitySparseSet<T>::const_iterator& begin, typename World::EntitySparseSet<T>::const_iterator& end){
    begin = staticEmptyContainer<T>.begin();
    end = staticEmptyContainer<T>.end();
}

void RavEngine::World::Tick(float scale) {
	
    PreTick(scale);
	
	//Tick the game code
	TickECS(scale);

    PostTick(scale);
}


RavEngine::World::World() : Solver(std::make_unique<PhysicsSolver>()){
    // init render data if the render engine is online
#if !RVE_SERVER

    if (GetApp() && GetApp()->GetDevice()) {
        renderData.emplace();
    }
#endif
    SetupTaskGraph();
    EmplacePolymorphicSystem<ScriptSystem>();
    EmplaceSystem<AnimatorSystem>();
	EmplaceSystem<SocketSystem>();
    CreateDependency<AnimatorSystem,ScriptSystem>();			// run scripts before animations
    CreateDependency<AnimatorSystem,PhysicsLinkSystemRead>();	// run physics reads before animator
    CreateDependency<PhysicsLinkSystemWrite,ScriptSystem>();	// run physics write before scripts
	CreateDependency<SocketSystem, AnimatorSystem>();			// run animator before socket system
#if !RVE_SERVER

    EmplaceSystem<AudioRoomSyncSystem>();
#endif
    EmplaceSystem<RPCSystem>();
#if !RVE_SERVER

    if (PHYSFS_isInit()){
        skybox = make_shared<Skybox>();
    }
#endif
}

void World::NetworkingSpawn(ctti_t id, Entity& handle){
    // are we networked, and the server?
    if (NetworkManager::IsNetworked() && NetworkManager::IsServer()){
        // is the constructed type a network object?
        if(GetApp()->networkManager.isNetworkEntity(id)){
            //add a network identity to this entity
            auto& netidcomp = handle.EmplaceComponent<NetworkIdentity>(id);
            
            // now send the message to spawn this on the other end
            GetApp()->networkManager.Spawn(this,id,handle.id,netidcomp.GetNetworkID());
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
            GetApp()->networkManager.Destroy(netidcomp.GetNetworkID());
        }
    }
}

/**
 Tick all of the objects in the world, multithreaded
 @param fpsScale the scale factor to apply to all operations based on the frame rate
 */
void RavEngine::World::TickECS(float fpsScale) {
	currentFPSScale = fpsScale;

	//update time
	time_now = e_clock_t::now();
	
	//execute and wait
    GetApp()->executor.run(masterTasks).wait();
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
	
#if !RVE_SERVER
    //TODO: FIX (use conditional tasking here)
    setupRenderTasks();
#endif
    
    ECSTasks.name("ECS");
    ECSTaskModule = masterTasks.composed_of(ECSTasks).name("ECS");
    
    // ensure Systems run before rendering
#if !RVE_SERVER
    renderTaskModule.succeed(ECSTaskModule);
#endif
    
    // process any dispatched coroutines
    auto updateAsyncIterators = ECSTasks.emplace([&]{
        async_begin = async_tasks.begin();
        async_end = async_tasks.end();
    }).name("async iterator update");
    auto doAsync = ECSTasks.for_each(std::ref(async_begin), std::ref(async_end), [&](const shared_ptr<dispatched_func>& item){
        if (GetApp()->GetCurrentTime() >= item->runAtTime){
            item->func();
            ranFunctions.push_back(async_tasks.hash_for(item));
        }
    }).name("Exec Async");
    updateAsyncIterators.precede(doAsync);
    auto cleanupRanAsync = ECSTasks.emplace([&]{
        // remove functions that have been run
        for(const auto hash : ranFunctions){
            async_tasks.erase_by_hash(hash);
        }
        ranFunctions.clear();
    }).name("Async cleanup");
    doAsync.precede(cleanupRanAsync);
    
    //add the PhysX tick, must run after write but before read

	auto physicsRootTask = ECSTasks.emplace([] {}).name("PhysicsRootTask");

	auto RunPhysics = ECSTasks.emplace([this]{
		Solver->Tick(GetCurrentFPSScale());
	}).name("PhysX Execute");
    
    auto read = EmplaceSystem<PhysicsLinkSystemRead>();
    auto write = EmplaceSystem<PhysicsLinkSystemWrite>();
    RunPhysics.precede(read.second);
    RunPhysics.succeed(write.second);
	
    physicsRootTask.precede(read.first,write.first);
	read.second.succeed(RunPhysics);	// if checkRunPhysics returns a 1, it goes here anyways.
    
#if !RVE_SERVER
    // setup audio tasks
    audioTasks.name("Audio");
    
    auto audioClear = audioTasks.emplace([this]{
        GetApp()->GetCurrentAudioSnapshot()->Clear();
        //TODO: currently this selects the LAST listener, but there is no need for this
        Filter([](const AudioListener& listener, const Transform& transform){
            auto ptr = GetApp()->GetCurrentAudioSnapshot();
            ptr->listenerPos = transform.GetWorldPosition();
            ptr->listenerRot = transform.GetWorldRotation();
            ptr->listenerGraph = listener.GetGraph();
        });
    }).name("Clear + Listener");
    
  
    
    auto copyAudios = audioTasks.emplace([this]{
        Filter([this](AudioSourceComponent& audioSource, const Transform& transform){
            GetApp()->GetCurrentAudioSnapshot()->sources.emplace(audioSource.GetPlayer(),transform.GetWorldPosition(),transform.GetWorldRotation());
        });
        
        // now clean up the fire-and-forget audios that have completed
        instantaneousToPlay.remove_if([](const InstantaneousAudioSource& ias){
            return ! ias.GetPlayer()->IsPlaying();
        });
        
        // now do fire-and-forget audios that need to play
        for(auto& f : instantaneousToPlay){
            GetApp()->GetCurrentAudioSnapshot()->sources.emplace(f.GetPlayer(),f.source_position,quaternion(0,0,0,1));
        }
    }).name("Point Audios").succeed(audioClear);
    
    auto copyAmbients = audioTasks.emplace([this]{
        // raster audio
        Filter([this](AmbientAudioSourceComponent& audioSource){
            GetApp()->GetCurrentAudioSnapshot()->ambientSources.emplace(audioSource.GetPlayer());
        });

        // now clean up the fire-and-forget audios that have completed
        ambientToPlay.remove_if([](const InstantaneousAmbientAudioSource& ias) {
            return !ias.GetPlayer()->IsPlaying();
        });
        
        // now do fire-and-forget audios that need to play
        for(auto& f : ambientToPlay){
            GetApp()->GetCurrentAudioSnapshot()->ambientSources.emplace(f.GetPlayer());
        }
        
    }).name("Ambient Audios").succeed(audioClear);
    
    auto copyRooms = audioTasks.emplace([this]{
        Filter( [this](AudioRoom& room, Transform& transform){
            GetApp()->GetCurrentAudioSnapshot()->rooms.emplace_back(room.data,transform.GetWorldPosition(),transform.GetWorldRotation());
        });
        
    }).name("Rooms").succeed(audioClear);
    
    auto audioSwap = audioTasks.emplace([]{
        GetApp()->SwapCurrrentAudioSnapshot();
    }).name("Swap Current").succeed(copyAudios,copyAmbients,copyRooms);
    
    audioTaskModule = masterTasks.composed_of(audioTasks).name("Audio");
    audioTaskModule.succeed(ECSTaskModule);
#endif
}
#if !RVE_SERVER

void World::setupRenderTasks(){
	//render engine data collector
	//camera matrices
    renderTasks.name("Render");
   
    auto resizeBuffer = renderTasks.emplace([this]{
        // can the world transform list hold that many objects?
        // to avoid an indirection, we assume all entities may have a transform
        // this wastes some VRAM
        auto nEntities = localToGlobal.size() + std::min(nCreatedThisTick, 1);  // hack: if I don't add 1, then the pbr.vsh shader OOBs, not sure why
        auto currentBufferSize = renderData->worldTransforms.size();
        if (nEntities > currentBufferSize){
            auto newSize = closest_power_of(nEntities, 16);
            renderData->worldTransforms.resize(newSize);
        }
        nCreatedThisTick = 0;
    });
    
    auto updateRenderDataGeneric = [this]<typename SM_T, typename ... Aux_T>(const SM_T* sm_t_holder, auto& renderDataSource, auto&& captureLambda, auto&& iteratorComparator, const Aux_T* ... axillaryParams){
        Filter([this,&renderDataSource,&captureLambda,&iteratorComparator](const SM_T& sm, const Aux_T& ..., Transform& trns) {
            if (trns.isTickDirty && sm.GetEnabled()) {
                // update
                assert(renderDataSource.contains(sm.GetMaterial()));
                auto valuesToCompare = captureLambda(sm);
                renderDataSource.if_contains(sm.GetMaterial(), [&trns,this, &iteratorComparator, &valuesToCompare](auto& row) {
                    auto it = iteratorComparator(row, valuesToCompare);
                    if (it == row.commands.end()){
                        return;
                    }
                    assert(it != row.commands.end());
                    auto& vec = *it;
                    // write new matrix
                    auto owner = trns.GetOwner();
                    auto ownerIDInWorld = owner.GetIdInWorld();
                    renderData->worldTransforms[ownerIDInWorld] = trns.GetWorldMatrix();
                });

                trns.ClearTickDirty();
            }
        });
    };

    auto updateRenderDataStaticMesh = renderTasks.emplace([this,updateRenderDataGeneric] {
        constexpr static StaticMesh* ptrForTemplate = nullptr;
        updateRenderDataGeneric(ptrForTemplate,renderData->staticMeshRenderData, [](auto& sm){
            return sm.GetMesh();
        }, [](auto& row, auto& meshToUpdate){
            return std::find_if(row.commands.begin(), row.commands.end(), [&](const auto& value) {
                return value.mesh.lock() == meshToUpdate;
            });
        });
       
    }).name("Update invalidated static mesh transforms");

    auto updateRenderDataSkinnedMesh = renderTasks.emplace([this,updateRenderDataGeneric] {
        constexpr static SkinnedMeshComponent* ptrForTemplate = nullptr;
        constexpr static AnimatorComponent* ptrForTemplate2 = nullptr;
        updateRenderDataGeneric(ptrForTemplate,renderData->skinnedMeshRenderData, [](auto& sm){
            return std::make_pair(sm.GetMesh(), sm.GetSkeleton());
        }, [](auto& row, auto& valuesToCompare){
            return std::find_if(row.commands.begin(), row.commands.end(), [&](const auto& value) {
                return value.mesh.lock() == valuesToCompare.first && value.skeleton.lock() == valuesToCompare.second;
            });
        }, ptrForTemplate2);
    }).name("Upate invalidated skinned mesh transforms");
    
    resizeBuffer.precede(updateRenderDataStaticMesh, updateRenderDataSkinnedMesh);
    
    auto updateInvalidatedDirs = renderTasks.emplace([this]{
        if (auto ptr = GetAllComponentsOfType<DirectionalLight>()){
            for(int i = 0; i < ptr->DenseSize(); i++){
                auto owner = Entity(localToGlobal[ptr->GetOwner(i)]);
                auto& transform = owner.GetTransform();
                if (transform.isTickDirty){
                    // update transform data if it has changed
                    auto rot = owner.GetTransform().WorldUp();

                    // use local ID here, no need for local-to-global translation
                    renderData->directionalLightData.uploadData.GetForSparseIndex(ptr->GetOwner(i)).direction = rot;
                }
                if (ptr->Get(i).isInvalidated()){
                    // update color data if it has changed
                    auto& lightdata = ptr->Get(i);
                    auto& color = lightdata.GetColorRGBA();
                    auto owner = ptr->GetOwner(i);
                    auto& dirLightUploadData = renderData->directionalLightData.uploadData.GetForSparseIndex(owner);
                    auto& dirLightAuxData = renderData->directionalLightData.auxData.GetForSparseIndex(owner);
                    dirLightUploadData.colorIntensity = {color.R, color.G, color.B, lightdata.GetIntensity()};
                    dirLightUploadData.castsShadows = lightdata.CastsShadows();
                    lightdata.clearInvalidate();
                    
                    dirLightAuxData.shadowDistance = lightdata.GetShadowDistance();
                }
                // don't reset transform tickInvalidated here because the meshUpdater needs it after this
            }
        }
    }).name("Update Invalidated DirLights").precede(updateRenderDataStaticMesh, updateRenderDataSkinnedMesh);
    
    auto updateInvalidatedSpots = renderTasks.emplace([this]{
        if (auto ptr = GetAllComponentsOfType<SpotLight>()){
            for(int i = 0; i < ptr->DenseSize(); i++){
                auto owner = Entity(localToGlobal[ptr->GetOwner(i)]);
                auto& transform = owner.GetTransform();
                if (transform.isTickDirty){
                    // update transform data if it has changed
                    renderData->spotLightData.uploadData.GetForSparseIndex(ptr->GetOwner(i)).worldTransform = transform.GetWorldMatrix();
                }
                if (ptr->Get(i).isInvalidated()){
                    // update color data if it has changed
                    auto& lightData = ptr->Get(i);
                    auto& colorData = lightData.GetColorRGBA();
                    auto& denseData = renderData->spotLightData.uploadData.GetForSparseIndex(ptr->GetOwner(i));
                    denseData.coneAndPenumbra = { lightData.GetConeAngle(), lightData.GetPenumbraAngle() };
                    denseData.colorIntensity = { colorData.R,colorData.G,colorData.B,lightData.GetIntensity()};
                    denseData.castsShadows = lightData.CastsShadows();
                    lightData.clearInvalidate();
                }
                // don't reset transform tickInvalidated here because the meshUpdater needs it after this
            }
        }
    }).name("Update Invalidated SpotLights").precede(updateRenderDataStaticMesh, updateRenderDataSkinnedMesh);
    
    auto updateInvalidatedPoints = renderTasks.emplace([this]{
        if (auto ptr = GetAllComponentsOfType<PointLight>()){
            for(int i = 0; i < ptr->DenseSize(); i++){
                auto owner = Entity(localToGlobal[ptr->GetOwner(i)]);
                auto& transform = owner.GetTransform();
                if (transform.isTickDirty){
                    // update transform data if it has changed
                    renderData->pointLightData.uploadData.GetForSparseIndex(ptr->GetOwner(i)).worldTransform = transform.GetWorldMatrix();
                }
                if (ptr->Get(i).isInvalidated()){
                    // update color data if it has changed
                    auto& lightData = ptr->Get(i);
                    auto& colorData = lightData.GetColorRGBA();
                    renderData->pointLightData.uploadData.GetForSparseIndex(ptr->GetOwner(i)).colorIntensity = { colorData.R,colorData.G,colorData.B,lightData.GetIntensity()};
                    ptr->Get(i).clearInvalidate();
                }
                // don't reset transform tickInvalidated here because the meshUpdater needs it after this
            }
        }
    }).name("Update Invalidated SpotLights").precede(updateRenderDataStaticMesh, updateRenderDataSkinnedMesh);
    
    auto updateInvalidatedAmbients = renderTasks.emplace([this]{
        if(auto ptr = GetAllComponentsOfType<AmbientLight>()){
            for(int i = 0; i < ptr->DenseSize(); i++){
                auto ownerLocalId = ptr->GetOwner(i);
                auto& light = ptr->Get(i);
                auto& color = light.GetColorRGBA();
                renderData->ambientLightData.uploadData.GetForSparseIndex(ownerLocalId) = {color.R, color.G, color.B, light.GetIntensity()};
                light.clearInvalidate();
            }
        }
    }).name("Update Invalidated AmbLights");

    
    // attatch the renderTasks module to the masterTasks
    renderTaskModule = masterTasks.composed_of(renderTasks).name("Render");
}
#endif

void World::DispatchAsync(const Function<void ()>& func, double delaySeconds){
    auto time = GetApp()->GetCurrentTime();
    GetApp()->DispatchMainThread([=]{
        async_tasks.insert(make_shared<dispatched_func>(time + delaySeconds,func));
    });
}
#if !RVE_SERVER
void DestroyMeshRenderDataGeneric(const auto& mesh, auto material, auto&& renderData, entity_t local_id, auto&& iteratorComparator){
    
    bool removeContains = false;
    auto data_it = renderData.find(material);
    if (data_it != renderData.end()){
        auto& data = (*data_it).second;
        auto it = std::find_if(data.commands.begin(), data.commands.end(), [&](auto& other) {
            return iteratorComparator(other);
        });
        if (it != data.commands.end() && (*it).entities.HasForSparseIndex(local_id)) {
            (*it).entities.EraseAtSparseIndex(local_id);
            // if empty, remove from the larger container
            if ((*it).entities.DenseSize() == 0) {
                data.commands.erase(it);
            }
            if (data.commands.size() == 0){
                removeContains = true;
            }
        }
    }
    if (removeContains){
        renderData.erase(material);
    }
    
}

void updateMeshMaterialGeneric(auto&& renderData, entity_t localID, auto oldMat, auto newMat, auto mesh, auto&& deletionComparator, auto&& comparator, auto&& newConstructionFunction){
    
    // detect the case of the material set to itself
    if (oldMat == newMat) {
        return;
    }

    // remove render data for the old mesh
    DestroyMeshRenderDataGeneric(mesh, oldMat, renderData, localID, deletionComparator);
        
    // add the new mesh & its transform to the hashmap
    auto& set = ( * (renderData.try_emplace(newMat, typename std::remove_reference_t<decltype(renderData)>::mapped_type()).first)).second;
    bool found = false;
    for (auto& command : set.commands) {
        found = comparator(command);
        if (found) {
            command.entities.Emplace(localID,localID);
            break;
        }
    }
    // otherwise create a new entry
    if (!found) {
        newConstructionFunction(set.commands);
    }
    
}

void RavEngine::World::updateStaticMeshMaterial(entity_t localId, decltype(RenderData::staticMeshRenderData)::key_type oldMat, decltype(RenderData::staticMeshRenderData)::key_type newMat, Ref<MeshAsset> mesh)
{
    // do nothing if renderer is not online
    if (!renderData) {
        return;
    }

    assert(HasComponent<Transform>(localId) && "Cannot change material on an entity that does not have a transform!");
    updateMeshMaterialGeneric(renderData->staticMeshRenderData, localId, oldMat, newMat, mesh,
        [mesh](auto&& other){
            return other.mesh.lock() == mesh;
        },
        [mesh](auto&& command){
            auto cmpMesh = command.mesh.lock();
            return cmpMesh == mesh;
        },
        [mesh, localId](auto&& commands){
            commands.emplace(mesh, localId, localId);
        }
    );
}

void RavEngine::World::updateSkinnedMeshMaterial(entity_t localId, decltype(RenderData::skinnedMeshRenderData)::key_type oldMat, decltype(RenderData::skinnedMeshRenderData)::key_type newMat, Ref<MeshAssetSkinned> mesh, Ref<SkeletonAsset> skeleton)
{
    // if render engine is not online, do nothing
    if (!renderData) {
        return;
    }
    
    assert(HasComponent<Transform>(localId) && "Cannot change material on an entity that does not have a transform!");
    updateMeshMaterialGeneric(renderData->skinnedMeshRenderData, localId, oldMat, newMat, mesh,
        [mesh, &skeleton](auto&& other){
            return other.mesh.lock() == mesh && other.skeleton.lock() == skeleton;
        },
        [mesh, &skeleton](auto&& command){
            auto cmpMesh = command.mesh.lock();
            return cmpMesh == mesh && command.skeleton.lock() == skeleton;
        },
        [mesh, &skeleton, localId](auto&& commands){
            commands.emplace(mesh, skeleton, localId, localId);
        }
    );
}

void RavEngine::World::DestroyStaticMeshRenderData(const StaticMesh& mesh, entity_t local_id)
{
    if (!renderData) {
        return;
    }
    
    auto meshData = mesh.GetMesh();
    DestroyMeshRenderDataGeneric(mesh.GetMesh(), mesh.GetMaterial(), renderData->staticMeshRenderData, local_id, [meshData](auto&& other){
        return other.mesh.lock() == meshData;
    });
}

void World::DestroySkinnedMeshRenderData(const SkinnedMeshComponent& mesh, entity_t local_id) {
    if (!renderData) {
        return;
    }
    
    auto skeleton = mesh.GetSkeleton();
    auto meshData = mesh.GetMesh();
    DestroyMeshRenderDataGeneric(mesh.GetMesh(), mesh.GetMaterial(), renderData->skinnedMeshRenderData, local_id, [&meshData, &skeleton](auto&& other){
        return other.mesh.lock() == meshData && other.skeleton.lock() == skeleton;
    });
}

void World::StaticMeshChangedVisibility(const StaticMesh* mesh){
	auto owner = mesh->GetOwner();
	if (mesh->GetEnabled()){
		updateStaticMeshMaterial(owner.GetIdInWorld(),nullptr,mesh->GetMaterial(),mesh->GetMesh());
	}
	else{
		DestroyStaticMeshRenderData(*mesh, owner.GetIdInWorld());
	}
}

void World::SkinnedMeshChangedVisibility(const SkinnedMeshComponent* mesh){
	auto owner = mesh->GetOwner();
	if (mesh->GetEnabled()){
		updateSkinnedMeshMaterial(owner.GetIdInWorld(),nullptr,mesh->GetMaterial(),mesh->GetMesh(),mesh->GetSkeleton());
	}
	else{
		DestroySkinnedMeshRenderData(*mesh, owner.GetIdInWorld());
	}
}
#endif


entity_t World::CreateEntity(){
    entity_t id;
    if (available.size() > 0){
        id = available.front();
        available.pop();
    }
    else{
        id = static_cast<decltype(id)>(localToGlobal.size());
        localToGlobal.push_back(INVALID_ENTITY);
        nCreatedThisTick++;
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
#if !RVE_SERVER
void RavEngine::World::PlaySound(const InstantaneousAudioSource& ias) {
    instantaneousToPlay.push_back(ias);
}

void RavEngine::World::PlayAmbientSound(const InstantaneousAmbientAudioSource& iaas) {
    ambientToPlay.push_back(iaas);
}
#endif

void World::DeallocatePhysics(){
    Solver->DeallocatePhysx();
}
#if !RVE_SERVER

RavEngine::World::MDICommandBase::~MDICommandBase()
{
    if (auto app = GetApp()) {
        auto& gcBuffers = app->GetRenderEngine().gcBuffers;
        gcBuffers.enqueue(indirectBuffer);
        gcBuffers.enqueue(cullingBuffer);
        gcBuffers.enqueue(indirectStagingBuffer);
    }
  
}
#endif
