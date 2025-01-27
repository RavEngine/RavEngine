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
#include "Profile.hpp"
#if !RVE_SERVER
    #include "VRAMSparseSet.hpp"
    #include "AudioMeshComponent.hpp"
    #include "ParticleEmitter.hpp"
    #include "MeshCollection.hpp"
#else
    #include "Transform.hpp"
#endif
#include "PhysicsBodyComponent.hpp"
#include "CaseAnalysis.hpp"

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
    RVE_PROFILE_FN;
    if (graphWasModified) {
        CheckSystems();
    }

    PreTick(scale);
	
	//Tick the game code
	TickECS(scale);

    PostTick(scale);

#if !RVE_SERVER
    // run the render sync tasks
    RVE_PROFILE_SECTION(render, "Sync render data");
    auto& executor = GetApp()->executor;
    executor.run(renderTasks).wait();
    RVE_PROFILE_SECTION_END(render);
#endif
}


RavEngine::World::World() : Solver(std::make_unique<PhysicsSolver>(this)){
    SetupTaskGraph();
    EmplacePolymorphicSystem<ScriptSystem>();
    EmplaceSystem<AnimatorSystem>();
	EmplaceSystem<SocketSystem>();
    CreateDependency<AnimatorSystem,ScriptSystem>();			// run scripts before animations
    CreateDependency<AnimatorSystem,PhysicsLinkSystemRead>();	// run physics reads before animator
    CreateDependency<PhysicsLinkSystemWrite,ScriptSystem>();	// run physics write before scripts
	CreateDependency<SocketSystem, AnimatorSystem>();			// run animator before socket system

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
            GetApp()->networkManager.Spawn(this,id,handle,netidcomp.GetNetworkID());
        }
    }
}

void World::NetworkingDestroy(entity_t id){
    Entity handle{id,this};
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
    GetApp()->executor.run(masterTasks);
    GetApp()->executor.wait_for_all();
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
    
    // process any dispatched coroutines
    auto updateAsyncIterators = ECSTasks.emplace([&]{
        async_begin = async_tasks.begin();
        async_end = async_tasks.end();
    }).name("async iterator update");
    auto doAsync = ECSTasks.for_each(std::ref(async_begin), std::ref(async_end), [&](const shared_ptr<dispatched_func>& item){
        if (GetApp()->GetCurrentTime() >= item->runAtTime){
            item->func();
            ranFunctions.push_back(item);
        }
    }).name("Exec Async");
    updateAsyncIterators.precede(doAsync);
    auto cleanupRanAsync = ECSTasks.emplace([&]{
        // remove functions that have been run
        for(const auto item : ranFunctions){
            async_tasks.erase(item);
        }
        ranFunctions.clear();
    }).name("Async cleanup");
    doAsync.precede(cleanupRanAsync);
    
    //add the PhysX tick, must run after write but before read

	auto physicsRootTask = ECSTasks.emplace([] {}).name("PhysicsRootTask");

	auto RunPhysics = ECSTasks.emplace([this]{
        RVE_PROFILE_FN_N("PhysX Tick");
		Solver->Tick(GetCurrentFPSScale());
	}).name("PhysX Execute");
    
    auto read = EmplaceSystem<PhysicsLinkSystemRead>();
    auto write = EmplaceSerialSystem<PhysicsLinkSystemWrite>();
    RunPhysics.precede(read.do_task);
    RunPhysics.succeed(write.do_task);
	
    physicsRootTask.precede(read.rangeUpdate,write.rangeUpdate);
	read.do_task.succeed(RunPhysics);	// if checkRunPhysics returns a 1, it goes here anyways.
    
#if !RVE_SERVER
        // setup audio tasks
        if (GetApp()->GetAudioActive()){
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
            GetApp()->GetCurrentAudioSnapshot()->sourceWorld = shared_from_this();
        }).name("Clear + Listener");
    
  
    
        auto copyAudios = audioTasks.emplace([this]{
            Filter([this](const AudioSourceComponent& audioSource, const Transform& transform){
                auto snapshot = GetApp()->GetCurrentAudioSnapshot();
                auto provider = audioSource.GetPlayer();
                snapshot->sources.emplace(provider,transform.GetWorldPosition(),transform.GetWorldRotation(), audioSource.GetOwner().GetID());
                snapshot->dataProviders.insert(provider);
            });
        
            // now clean up the fire-and-forget audios that have completed
            constexpr auto checkFunc = [](const InstantaneousAudioSourceToPlay& ias) {
                return !ias.source.GetPlayer()->IsPlaying();
            };
            for (const auto& source : instantaneousToPlay) {
                if (checkFunc(source)) {
                    destroyedAudioSources.enqueue(source.fakeOwner.id.id);
                    instantaneousAudioSourceFreeList.ReturnID(source.fakeOwner.id);    // expired sources return their IDs
                }
            }
            instantaneousToPlay.remove_if(checkFunc);

        
            // now do fire-and-forget audios that need to play
            for(auto& f : instantaneousToPlay){
                auto snapshot = GetApp()->GetCurrentAudioSnapshot();
                auto provider = f.source.GetPlayer();
                snapshot->sources.emplace(provider,f.source.source_position,quaternion(0,0,0,1), f.fakeOwner.GetID());
                snapshot->dataProviders.insert(provider);
            }
        }).name("Point Audios").succeed(audioClear);
    
        auto copyAmbients = audioTasks.emplace([this]{
            // raster audio
            Filter([this](const AmbientAudioSourceComponent& audioSource){
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
    
        auto copySimpleAudioSpaces = audioTasks.emplace([this]{
            Filter( [this](const SimpleAudioSpace& room, const Transform& transform){
                GetApp()->GetCurrentAudioSnapshot()->simpleAudioSpaces.emplace_back(room.GetData(), transform.GetWorldPosition());
            });
        
        }).name("Simple Audio Spaces").succeed(audioClear);

        auto copyGeometryAudioSpaces = audioTasks.emplace([this] {
            Filter([this](const GeometryAudioSpace& room, const Transform& transform) {
                GetApp()->GetCurrentAudioSnapshot()->geometryAudioSpaces.emplace_back(room.GetData(), transform.GetWorldPosition(), glm::inverse(transform.GetWorldMatrix()));
            });

        }).name("Geometry Audio Spaces").succeed(audioClear);

        auto copyAudioGeometry = audioTasks.emplace([this] {
            Filter([this](const AudioMeshComponent& mesh, const Transform& transform) {
                GetApp()->GetCurrentAudioSnapshot()->audioMeshes.emplace_back(transform.GetWorldMatrix(), mesh.GetAsset(), mesh.GetOwner().GetID());
            });
         }).name("Geometry Audio Meshes").succeed(audioClear);

         auto copyAudioBoxSpaces = audioTasks.emplace([this] {
             Filter([this](const BoxReverbationAudioSpace& room, const Transform& transform) {
                 GetApp()->GetCurrentAudioSnapshot()->boxAudioSpaces.emplace_back(room.GetData(), transform.GetWorldMatrix(), room.GetHalfExts(), room.GetRoomProperties());
             });
        }).name("Box Reverb Audio Meshes").succeed(audioClear);
    
        auto audioSwap = audioTasks.emplace([]{
            GetApp()->SwapCurrrentAudioSnapshot();
        }).name("Swap Current").succeed(copyAudios,copyAmbients,copySimpleAudioSpaces,copyGeometryAudioSpaces, copyAudioGeometry, copyAudioBoxSpaces);
    
        audioTaskModule = masterTasks.composed_of(audioTasks).name("Audio");
        audioTaskModule.succeed(ECSTaskModule);
    }
#endif
}

World::EntityRedir::operator Entity() const{
    return {id, owner};
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
        auto nEntities = numEntities + std::min(nCreatedThisTick, 1);  // hack: if I don't add 1, then the pbr.vsh shader OOBs, not sure why
        auto currentBufferSize = renderData.worldTransforms.Size();
        if (nEntities > currentBufferSize){
            auto newSize = closest_power_of<entity_id_t>(nEntities, 16);
            renderData.worldTransforms.Resize(newSize);
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
                    auto ownerIDInWorld = owner.GetID();
                    renderData.worldTransforms.SetValueAt(ownerIDInWorld.id,trns.GetWorldMatrix());
                });

                trns.ClearTickDirty();
            }
        });
    };

    auto updateRenderDataStaticMesh = renderTasks.emplace([this,updateRenderDataGeneric] {
        constexpr static StaticMesh* ptrForTemplate = nullptr;
        updateRenderDataGeneric(ptrForTemplate,renderData.staticMeshRenderData, [](auto& sm){
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
        updateRenderDataGeneric(ptrForTemplate,renderData.skinnedMeshRenderData, [](auto& sm){
            return std::make_pair(sm.GetMesh(), sm.GetSkeleton());
        }, [](auto& row, auto& valuesToCompare){
            return std::find_if(row.commands.begin(), row.commands.end(), [&](const auto& value) {
                return value.mesh.lock() == valuesToCompare.first && value.skeleton.lock() == valuesToCompare.second;
            });
        }, ptrForTemplate2);
    }).name("Upate invalidated skinned mesh transforms");

    auto updateParticleSystems = renderTasks.emplace([this] {
        Filter([this](const ParticleEmitter& emitter, const Transform& t) {
            if (t.getTickDirty()) {
                auto owner = t.GetOwner();
                auto ownerIDInWorld = owner.GetID();
                renderData.worldTransforms.SetValueAt(ownerIDInWorld.id, t.GetWorldMatrix());
            }
        });
    });
    
    resizeBuffer.precede(updateRenderDataStaticMesh, updateRenderDataSkinnedMesh, updateParticleSystems);
    
    auto updateInvalidatedDirs = renderTasks.emplace([this]{
        if (auto ptr = GetAllComponentsOfType<DirectionalLight>()){
            for(entity_id_t i = 0; i < ptr->DenseSize(); i++){
                auto ownerID = ptr->GetOwner(i);
                auto owner = Entity({ownerID, VersionForEntity(ownerID)},this);
                auto& transform = owner.GetTransform();
                if (transform.isTickDirty){
                    // update transform data if it has changed
                    auto rot = owner.GetTransform().WorldUp();

                    // use local ID here, no need for local-to-global translation
                    auto& uploadData = renderData.directionalLightData.GetForSparseIndexForWriting(ptr->GetOwner(i));
                    uploadData.direction = rot;
                }
                auto& lightdata = ptr->Get({i});
                if (lightdata.isInvalidated()) {
                    // update color data if it has changed
                    auto& color = lightdata.GetColorRGBA();
                    auto owner = ptr->GetOwner(i);
                    auto& dirLightUploadData = renderData.directionalLightData.GetForSparseIndexForWriting(owner);
                    dirLightUploadData.color = {color.R, color.G, color.B};
                    dirLightUploadData.intensity = lightdata.GetIntensity();
                    dirLightUploadData.castsShadows = lightdata.CastsShadows();
                    for(uint8_t i = 0; i < MAX_CASCADES; i++){
                        dirLightUploadData.shadowmapBindlessIndex[i] = lightdata.shadowData.shadowMap[i]->GetDefaultView().GetReadonlyBindlessTextureHandle();
                    }
                    dirLightUploadData.shadowLayers = lightdata.GetShadowLayers();
                    dirLightUploadData.illuminationLayers = lightdata.GetIlluminationLayers();
                    dirLightUploadData.numCascades = lightdata.numCascades;
                    lightdata.clearInvalidate();
                    
                }
                // don't reset transform tickInvalidated here because the meshUpdater needs it after this
            }
        }
    }).name("Update Invalidated DirLights").precede(updateRenderDataStaticMesh, updateRenderDataSkinnedMesh);
    
    auto updateInvalidatedSpots = renderTasks.emplace([this]{
        if (auto ptr = GetAllComponentsOfType<SpotLight>()){
            for(entity_id_t i = 0; i < ptr->DenseSize(); i++){
                auto ownerID = ptr->GetOwner(i);
                auto owner = Entity({ownerID, VersionForEntity(ownerID)},this);
                auto& transform = owner.GetTransform();
                auto& lightData = ptr->Get({i});
                if (transform.isTickDirty){
                    // update transform data if it has changed
                    auto& denseData = renderData.spotLightData.GetForSparseIndexForWriting(ptr->GetOwner(i));
                    denseData.worldTransform = transform.GetWorldMatrix();
                    
                    const auto proj = lightData.CalcProjectionMatrix();
                    const auto view = lightData.CalcViewMatrix(denseData.worldTransform);
                    denseData.lightViewProj = proj * view;
                }
                if (lightData.isInvalidated()){
                    // update color data if it has changed
                    auto& colorData = lightData.GetColorRGBA();
                    auto& denseData = renderData.spotLightData.GetForSparseIndexForWriting(ptr->GetOwner(i));
                    denseData.coneAngle = lightData.GetConeAngle();
                    denseData.penumbraAngle = lightData.GetPenumbraAngle();
                    denseData.color = { colorData.R,colorData.G,colorData.B};
                    denseData.intensity = lightData.GetIntensity();
                    denseData.castsShadows = lightData.CastsShadows();
                    denseData.shadowmapBindlessIndex = lightData.shadowData.shadowMap->GetDefaultView().GetReadonlyBindlessTextureHandle();
                    denseData.shadowLayers = lightData.GetShadowLayers();
                    denseData.illuminationLayers = lightData.GetIlluminationLayers();
                    lightData.clearInvalidate();
                }
                // don't reset transform tickInvalidated here because the meshUpdater needs it after this
            }
        }
    }).name("Update Invalidated SpotLights").precede(updateRenderDataStaticMesh, updateRenderDataSkinnedMesh);
    
    auto updateInvalidatedPoints = renderTasks.emplace([this]{
        if (auto ptr = GetAllComponentsOfType<PointLight>()){
            for(entity_id_t i = 0; i < ptr->DenseSize(); i++){
                auto ownerID = ptr->GetOwner(i);
                auto owner = Entity({ownerID, VersionForEntity(ownerID)},this);
                auto& transform = owner.GetTransform();
                if (transform.isTickDirty){
                    // update transform data if it has changed
                    auto& gpudata = renderData.pointLightData.GetForSparseIndexForWriting(ptr->GetOwner(i));
                    gpudata.position = transform.GetWorldPosition();
                }
                auto& lightData = ptr->Get({i});
                if (lightData.isInvalidated()){
                    // update color data if it has changed
                    
                    auto& colorData = lightData.GetColorRGBA();
                    auto& denseData = renderData.pointLightData.GetForSparseIndexForWriting(ptr->GetOwner(i));
                    denseData.color = { colorData.R,colorData.G,colorData.B};
                    denseData.intensity = lightData.GetIntensity();
                    denseData.castsShadows = lightData.CastsShadows();
                    denseData.shadowmapBindlessIndex = lightData.shadowData.mapCube->GetDefaultView().GetReadonlyBindlessTextureHandle();
                    denseData.shadowLayers = lightData.GetShadowLayers();
                    denseData.illuminationLayers = lightData.GetIlluminationLayers();
                    lightData.clearInvalidate();
                }
                // don't reset transform tickInvalidated here because the meshUpdater needs it after this
            }
        }
    }).name("Update Invalidated PointLights").precede(updateRenderDataStaticMesh, updateRenderDataSkinnedMesh);
    
    auto updateInvalidatedAmbients = renderTasks.emplace([this]{
        if(auto ptr = GetAllComponentsOfType<AmbientLight>()){
            for(entity_id_t i = 0; i < ptr->DenseSize(); i++){
                auto ownerLocalId = ptr->GetOwner(i);
                auto& light = ptr->Get({i});
                auto& color = light.GetColorRGBA();
                renderData.ambientLightData.GetForSparseIndexForWriting(ownerLocalId) = {{color.R, color.G, color.B}, light.GetIntensity(), light.GetIlluminationLayers()};
                light.clearInvalidate();
            }
        }
    }).name("Update Invalidated AmbLights"); 
}
#endif

void World::DispatchAsync(const Function<void ()>& func, double delaySeconds){
    auto time = GetApp()->GetCurrentTime();
    GetApp()->DispatchMainThread([=]{
        async_tasks.insert(make_shared<dispatched_func>(time + delaySeconds,func));
    });
}
#if !RVE_SERVER
void World::SetupPerEntityRenderData(entity_t localID){
    auto& renderLayers = renderData.renderLayers;
    auto& perObjectAttributes = renderData.perObjectAttributes;
    if (renderLayers.Size() <= localID.id){
        auto newSize = closest_power_of<entity_id_t>(localID.id+1, 2);
        renderLayers.Resize(newSize);
        perObjectAttributes.Resize(newSize);
    }
    renderLayers.SetValueAt(localID.id, ALL_LAYERS);
    perObjectAttributes.SetValueAt(localID.id, ALL_ATTRIBUTES);
}

void World::SetEntityRenderlayer(entity_t localid, renderlayer_t layers){
    renderData.renderLayers.SetValueAt(localid.id, layers);
}

void World::SetEntityAttributes(entity_t localid, perobject_t attributes)
{
    renderData.perObjectAttributes.SetValueAt(localid.id, attributes);
}

perobject_t World::GetEntityAttributes(entity_t localid)
{
    return renderData.perObjectAttributes[localid.id];
}

void DestroyMeshRenderDataGeneric(const auto& mesh, auto material, auto&& renderData, entity_t local_id, auto&& iteratorComparator){
    
    bool removeContains = false;
    auto data_it = renderData.find(material);
    if (data_it != renderData.end()){
        auto& data = (*data_it).second;
        auto it = std::find_if(data.commands.begin(), data.commands.end(), [&](auto& other) {
            return iteratorComparator(other);
        });
        if (it != data.commands.end() && (*it).entities.HasForSparseIndex(local_id.id)) {
            (*it).entities.EraseAtSparseIndex(local_id.id);
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
            command.entities.Emplace(localID.id,entity_id_t(localID.id));
            break;
        }
    }
    // otherwise create a new entry
    if (!found) {
        newConstructionFunction(set.commands);
    }
    
}

void RavEngine::World::updateStaticMeshMaterial(entity_t localId, Ref<MaterialInstance> oldMat, Ref<MaterialInstance> newMat, Ref<MeshCollectionStatic> mesh)
{

    assert(HasComponent<Transform>(localId) && "Cannot change material on an entity that does not have a transform!");
    updateMeshMaterialGeneric(renderData.staticMeshRenderData, localId, oldMat, newMat, mesh,
        [mesh](auto&& other){
            return other.mesh.lock() == mesh;
        },
        [mesh](auto&& command){
            auto cmpMesh = command.mesh.lock();
            return cmpMesh == mesh;
        },
        [mesh, localId](auto&& commands){
            commands.emplace(mesh, localId.id, localId.id);
        }
    );
}

void RavEngine::World::updateSkinnedMeshMaterial(entity_t localId, Ref<MaterialInstance> oldMat, Ref<MaterialInstance> newMat, Ref<MeshCollectionSkinned> mesh, Ref<SkeletonAsset> skeleton)
{
    
    assert(HasComponent<Transform>(localId) && "Cannot change material on an entity that does not have a transform!");
    updateMeshMaterialGeneric(renderData.skinnedMeshRenderData, localId, oldMat, newMat, mesh,
        [mesh, &skeleton](auto&& other){
            return other.mesh.lock() == mesh && other.skeleton.lock() == skeleton;
        },
        [mesh, &skeleton](auto&& command){
            auto cmpMesh = command.mesh.lock();
            return cmpMesh == mesh && command.skeleton.lock() == skeleton;
        },
        [mesh, &skeleton, localId](auto&& commands){
            commands.emplace(mesh, skeleton, localId.id, localId.id);
        }
    );
}

void RavEngine::World::DestroyStaticMeshRenderData(const StaticMesh& mesh, entity_t local_id)
{
    
    auto meshData = mesh.GetMesh();
    DestroyMeshRenderDataGeneric(mesh.GetMesh(), mesh.GetMaterial(), renderData.staticMeshRenderData, local_id, [meshData](auto&& other){
        return other.mesh.lock() == meshData;
    });
}

void World::DestroySkinnedMeshRenderData(const SkinnedMeshComponent& mesh, entity_t local_id) {
    
    auto skeleton = mesh.GetSkeleton();
    auto meshData = mesh.GetMesh();
    DestroyMeshRenderDataGeneric(mesh.GetMesh(), mesh.GetMaterial(), renderData.skinnedMeshRenderData, local_id, [&meshData, &skeleton](auto&& other){
        return other.mesh.lock() == meshData && other.skeleton.lock() == skeleton;
    });
}

void World::StaticMeshChangedVisibility(const StaticMesh* mesh){
	auto owner = mesh->GetOwner();
	if (mesh->GetEnabled()){
        updateStaticMeshMaterial(owner.GetID(),{},mesh->GetMaterial(),mesh->GetMesh());
	}
	else{
		DestroyStaticMeshRenderData(*mesh, owner.GetID());
	}
}

void World::SkinnedMeshChangedVisibility(const SkinnedMeshComponent* mesh){
	auto owner = mesh->GetOwner();
	if (mesh->GetEnabled()){
        updateSkinnedMeshMaterial(owner.GetID(),{},mesh->GetMaterial(),mesh->GetMesh(),mesh->GetSkeleton());
	}
	else{
		DestroySkinnedMeshRenderData(*mesh, owner.GetID());
	}
}
#endif


entity_t World::CreateEntity(){
    entity_t id;
    if (available.size() > 0){
        id.id = available.front();
        available.pop();
        id.version = versions[id.id];
    }
    else{
        id.id = numEntities++;
        nCreatedThisTick++;
        if (id.id >= versions.size()){
            versions.resize(closest_power_of<entity_id_t>(id.id + 1, 2));
            versions[id.id] = 0;
        }
        id.version = 0;
    }
    return id;
}

World::~World() {
#if ENABLE_RINGBUFFERS
    // dump out any live rooms
    Filter([this](const SimpleAudioSpace& space) {
        space.GetData()->OutputSampleData(Filesystem::CurrentWorkingDirectory() / (std::to_string(space.GetOwner().id.id) + ".wav"));
    });
#endif

    for(entity_id_t i = 0; i < numEntities; i++){
        if (EntityIsValid(i)){
            DestroyEntity({i, VersionForEntity(i)}); // destroy takes a local ID
        }
    }

}
#if !RVE_SERVER
void RavEngine::World::PlaySound(const InstantaneousAudioSource& ias) {
    instantaneousToPlay.emplace_back(ias,instantaneousAudioSourceFreeList.GetNextID());
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

void World::CheckSystems() {
#if 0
    auto findTaskOwner = [this](const tf::Task& task) -> std::optional<ctti_t> {
        for (const auto& [type, task] : typeToSystem) {
            if (task.do_task == task 
                || task.postHook == task
                || task.preHook == task
                || task.rangeUpdate == task
                ) {
                return type;
            }
    
        }
        return {};
    };
#endif
    struct CheckTaskPassed {};
    struct CheckTaskHooksFailure{};
    struct CheckTaskWorldDataProvider {};
    struct CheckTasksQueryFailure { ctti_t conflict; };
    using CheckTaskResult = std::variant<CheckTaskPassed, CheckTaskHooksFailure, CheckTaskWorldDataProvider, CheckTasksQueryFailure>;

    auto recurse_subtree = [](const tf::Task& root, auto&& fn) -> void {

        auto recurse_subtree_impl = [&fn](const tf::Task& root, auto&& recurse_subtree_fn) -> void {
            root.for_each_successor([&fn, &recurse_subtree_fn](const tf::Task& task) {
                fn(task);
                recurse_subtree_fn(task, recurse_subtree_fn);
             });
        };

        recurse_subtree_impl(root, recurse_subtree_impl);
    };

    auto checkTask = [this, &recurse_subtree](const SystemTasks& task1, const SystemTasks& task2) -> CheckTaskResult {
        // check task1 subtree
        bool dependencyExists = false;
        recurse_subtree(task1.rangeUpdate, [&task2,&dependencyExists](const tf::Task& successor1) {
            // are any of Task 2's tasks in Task 1's subtree?
            if (successor1 == task2.do_task) {
                dependencyExists = true;
            }
        });

        // check task2 subtree
        recurse_subtree(task2.rangeUpdate, [&task1, &dependencyExists](tf::Task successor2) {
            if (successor2 == task1.do_task) {
                dependencyExists = true;
            }
        });

        // if there is a dependency, then we know these two systems
        // cannot run at the same time, so we don't need to check them.
        if (dependencyExists) {
            return CheckTaskPassed{};
        }

        // if there is not a dependency, then these systems could execute
        // in parallel so we must check them.

        // check 0: does one of the systems have a pre or post hook? if it does, this situation
        // is unsafe because the hooks have arbitrary world access.
        if (task1.preHook.has_value() || task1.postHook.has_value() || task2.preHook.has_value() || task2.postHook.has_value()) {
            return CheckTaskHooksFailure{};
        }

        // does one of the systems use the WorldDataProvider? if so, it must be run in isolation
        if (task1.usesWorldDataProvider || task2.usesWorldDataProvider) {
            return CheckTaskWorldDataProvider{};
        }

        // check 1: do the queries overlap (can the systems operate on the same entities at the same time). 
        // If they do not, then these systems are safe to run in parallel.
        // A overlaps with B if all of the component types in A's query are in B's query and A does not have query types unique to it
        auto checkOverlap = [this](const SystemTasks& A, const SystemTasks& B) -> bool {
            uint32_t overlapCount = 0;
            UnorderedSet<ctti_t> alreadyTested;
            const auto testSet = [&alreadyTested,&overlapCount,&B, this](auto&& dependencies) {
                for (const auto id : dependencies) {  
                    const auto lookingFor = typeToName.at(id);
                    auto testDeplist = [&alreadyTested,&overlapCount](auto&& dependenciesList, ctti_t id) {
                        if (std::find(dependenciesList.begin(), dependenciesList.end(), id) != dependenciesList.end()) {
                            if (not alreadyTested.contains(id)) {
                                //continue;
                                overlapCount++;
                                alreadyTested.insert(id);
                            }
                        }
                    };
                    testDeplist(B.readDependencies, id);
                    testDeplist(B.writeDependencies, id);
                }
            };
            testSet(A.readDependencies);
            testSet(A.writeDependencies);
            
            if (overlapCount == A.readDependencies.size() + A.writeDependencies.size()) {
                // overlap detected!
                return true;
            }
            return false;
        };
        bool overlap = checkOverlap(task1, task2) || checkOverlap(task2, task1);

        // if there's no overlap, these are fine to run in parallel.
        if (!overlap) {
            return CheckTaskPassed{};
        }

        // check 2: For the overlap, is B reading or writing a component type that A is writing to?
        auto checkWriteOverlap = [](const SystemTasks& A, const SystemTasks& B) -> std::optional<ctti_t> {
            for (const auto& id : A.writeDependencies) {
                if (std::find(B.readDependencies.begin(), B.readDependencies.end(), id) != B.readDependencies.end()) {
                    return id;
                }
                if (std::find(B.readDependencies.begin(), B.readDependencies.end(), id) != B.readDependencies.end()) {
                    return id;
                }
            }
            return {};
        };
        auto res_a = checkWriteOverlap(task1, task2);
        auto res_b = checkWriteOverlap(task2, task1);

        if (auto id = res_a) {
            return CheckTasksQueryFailure{ id.value()};
        }
        if (auto id = res_b) {
            return CheckTasksQueryFailure{id.value()};
        }
        return CheckTaskPassed{};
    };

    for (const auto& [type,task] : typeToSystem) {
        for (const auto& [type2, task2] : typeToSystem) {
            auto sysName1 = typeToName.at(type);
            auto sysName2 = typeToName.at(type2);
            if (type == type2) {
                continue;
            }
            auto result = checkTask(task, task2);

            std::visit(
                CaseAnalysis{
                    [](const CheckTaskPassed&) {},
                    [&sysName1,&sysName2](const CheckTaskHooksFailure&) {
                        Debug::Fatal("{} and {} require an explicit dependency because one or both contains a pre or post hook", sysName1, sysName2);
                    },
                    [&sysName1,&sysName2](const CheckTaskWorldDataProvider&) {
                        Debug::Fatal("{} or {} uses a WorldDataProvider, and must be run in isolation", sysName1, sysName2);
                    },
                    [this,&sysName1,&sysName2](const CheckTasksQueryFailure& info) {
                        ExportTaskGraph(std::cerr);
                        auto typeName = typeToName.at(info.conflict);
                        Debug::Fatal("{} and {} access {} in an unsafe way!", sysName1, sysName2, typeName);
                    },
                },
                result);

        }
    }

    graphWasModified = false;
}
