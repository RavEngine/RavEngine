#pragma once

#include "mathtypes.hpp"
#include "Ref.hpp"
#include "ComponentWithOwner.hpp"
#include "Queryable.hpp"
#if !RVE_SERVER

#include "AudioSource.hpp"
#include "Types.hpp"
#include "DebugDrawer.hpp"
#include "AudioRingbuffer.hpp"
#include "Filesystem.hpp"
#include <api/resonance_audio_api.h>
#include <common/room_properties.h>

struct _IPLBinauralEffect_t;
struct _IPLDirectEffect_t;
struct _IPLSource_t;
struct _IPLSimulator_t;
struct _IPLScene_t;
struct _IPLInstancedMesh_t;
struct _IPLPathEffect_t;

namespace RavEngine{

class AudioRoomSyncSystem;
class AudioPlayer;
struct AudioMeshAsset;

using RoomMat = vraudio::MaterialName;

#define ENABLE_RINGBUFFERS 0

/**
 Renders audio buffers based on its owning world's state
 */
class SimpleAudioSpace : public ComponentWithOwner, public IDebugRenderable, public Queryable<SimpleAudioSpace,IDebugRenderable>{
	friend class RavEngine::AudioPlayer;
public:
    struct RoomData : public AudioGraphComposed{
        friend class RavEngine::AudioPlayer;
        float sourceRadius = 10;
        
        /**
        @param buffer destination for the calculated audio
        @param scratchBuffer scratch memory for effect graphs
        @param monoSourceData the point source data. Must be AudioRoom::NFRAMES in length and represent MONO audio
        @param sourcePos the position of the audio source in world space
        @param owningentity the owning ID of the audio source
        @param invListenerTransform the inverse of the listener transform in world space
        */
        void RenderAudioSource(
            PlanarSampleBufferInlineView& buffer, PlanarSampleBufferInlineView& scratchBuffer,
            PlanarSampleBufferInlineView monoSourceData, const vector3& sourcePos, entity_t owningEntity,
            const matrix4& invListenerTransform
        );

        // internal use only. Called when an audio source component is destroyed
        void DeleteAudioDataForEntity(entity_t entity);

        RoomData();
        ~RoomData();
#if ENABLE_RINGBUFFERS
        auto& GetRingBuffer(){
            return debugBuffer;
        }
        void OutputSampleData(const Filesystem::Path& path) const;
#endif
        
    private:
        struct SteamAudioEffects {
            _IPLBinauralEffect_t* binauralEffect = nullptr;
            _IPLDirectEffect_t* directEffect = nullptr;
        };

        void DestroyEffects(SteamAudioEffects&);

        locked_hashmap<entity_t, SteamAudioEffects,SpinLock> steamAudioData;

        SingleAudioRenderBuffer workingBuffers;
        SingleAudioRenderBufferNoScratch accumulationBuffer;

#if ENABLE_RINGBUFFERS
        AudioRingbuffer debugBuffer;
#endif
    };
	
    
	SimpleAudioSpace(Entity owner) : ComponentWithOwner(owner), data(std::make_shared<RoomData>()) {}
	
    void SetRadius(float radius) {
        data->sourceRadius = radius;
    }
    float GetRadius() const {
        return data->sourceRadius;
    }

    const auto GetData() const {
        return data;
    }
	
	/**
	 Render the debug shape for this room. Invoke in a debug rendering component
	 */
    void DebugDraw(RavEngine::DebugDrawer& dbg, const RavEngine::Transform& tr) const override {}
private:
    Ref<RoomData> data;
};

struct GeometryAudioSpace : public ComponentWithOwner, public IDebugRenderable, public Queryable<GeometryAudioSpace, IDebugRenderable> {
    friend class RavEngine::AudioRoomSyncSystem;
    friend class RavEngine::AudioPlayer;
public:

    struct RoomData : public AudioGraphComposed {
        friend class RavEngine::AudioPlayer;
        friend class GeometryAudioSpace;
        friend class AudioSnapshot;
        RoomData();
        ~RoomData();

        /**
        Present an audio source to the room. Depending on its position, it may or may not be added or removed from the room
        @param sourcePos the world-space position of the audio source
        @param owningEntity the world-local entity ID of the source
        @param roomPos the world-space position of the audio space
        @param invRoomTransform the inverse of the room's world-space transformation matrix
        */
        void ConsiderAudioSource(
            const vector3& sourcePos, entity_t owningEntity, const vector3& roomPos,
            const matrix4& invRoomTransform);

        /**
        Compute the Effect parameters for this room
        @param invRoomTranform the inverse of the room's world-space transformation matrix
        @param listenerForwardWorldSpace the forward vector for the listener in world space
        @param listenerUpWorldSpace the up vector for the listener in world space
        @param listenerRightWorldSpace the right vector for the listener in world space
        */
        void CalculateRoom(const matrix4& invRoomTransform, const vector3& listenerForwardWorldSpace, const vector3& listenerUpWorldSpace, const vector3& listenerRightWorldSpace);

        /**
        Present a mesh occluder to the room. Depending on its position, it may or may not be added or removed from the room
        @param mesh the asset representing the mesh
        @param transform the world-space transform of the mesh 
        @param roomPos the world-space position of the room
        @param invRoomTransform the inverse of the world-space transformation matrix for the room
        @param ownerID the world-local owner ID for the mesh
        */
        void ConsiderMesh(Ref<AudioMeshAsset> mesh, const matrix4& transform, const vector3& roomPos, const matrix4& invRoomTransform, entity_t ownerID);

        void RenderAudioSource(
            PlanarSampleBufferInlineView& outBuffer, PlanarSampleBufferInlineView& scratchBuffer,
            entity_t sourceOwningEntity, PlanarSampleBufferInlineView monoSourceData, const matrix4& invListenerTransform
        );

        // internal use only. Called when an audio source component is destroyed
        void DeleteAudioDataForEntity(entity_t entity);
        void DeleteMeshDataForEntity(entity_t entity);
    private:
        float sourceRadius = 10, meshRadius = 10;

        struct SteamAudioSourceConfig {
            _IPLSource_t* source = nullptr;
            _IPLDirectEffect_t* directEffect = nullptr;
            _IPLPathEffect_t* pathEffect = nullptr;
            _IPLBinauralEffect_t* binauralEffect = nullptr; //NOTE: this will be replaced by pathEffect at some point
            vector3 roomSpacePos{ 0,0,0 };
        };

        // must be called when destroying a SteamAudioSourceConfig
        void DestroySteamAudioSourceConfig(SteamAudioSourceConfig&);

        locked_hashmap<entity_t, SteamAudioSourceConfig, SpinLock> steamAudioSourceData;

        struct SteamAudioMeshConfig {
            _IPLInstancedMesh_t* instancedMesh = nullptr;
            vector3 lastPos{ 0,0,0 };
            quaternion lastRot{ 0,0,0,0 };
        };

        // must be called when destroying a SteamAudioMeshConfig
        void DestroySteamAudioMeshConfig(SteamAudioMeshConfig&);

        locked_hashmap<entity_t, SteamAudioMeshConfig, SpinLock> steamAudioMeshData;

        _IPLSimulator_t* steamAudioSimulator = nullptr;
        _IPLScene_t* rootScene = nullptr;

        SingleAudioRenderBuffer workingBuffers;
        SingleAudioRenderBufferNoScratch accumulationBuffer;
    };

    const auto GetData() const {
        return data;
    }

    /**
    * Sets the radius where audio sources will be included in the simulation
    */
    void SetAudioSourceRadius(float radius) {
        data->sourceRadius = radius;
    }

    /**
   * Sets the radius where audio mesh occluders will be included in the simulation
   */
    void SetMeshRadius(float radius) {
        data->meshRadius = radius;
    }

    auto GetAudioSourceRadius() const {
        return data->sourceRadius;
    }

    auto GetMeshRadius() const {
        return data->meshRadius;
    }

    GeometryAudioSpace(Entity owner) : ComponentWithOwner(owner), data(std::make_shared<RoomData>()) {}

    void DebugDraw(RavEngine::DebugDrawer& dbg, const RavEngine::Transform& tr) const override {}
private:
    Ref<RoomData> data;
};

class BoxReverbationAudioSpace : public ComponentWithOwner, public IDebugRenderable, public Queryable<BoxReverbationAudioSpace, IDebugRenderable> {
public:
    struct RoomProperties {
        float reflection_scalar = 1, reverb_gain = 1, reverb_time = 1, reverb_brightness = 0;
    };
private:

    friend class AudioPlayer;
    friend class AudioSnapshot;
    struct RoomData : public AudioGraphComposed {
        friend class AudioPlayer;

        // Material name of each surface of the shoebox room in this order:
        // [0] (-)ive x-axis wall (left)
        // [1] (+)ive x-axis wall (right)
        // [2] (-)ive y-axis wall (bottom)
        // [3] (+)ive y-axis wall (top)
        // [4] (-)ive z-axis wall (front)
        // [5] (+)ive z-axis wall (back)

        Array<RoomMat, 6> wallMaterials{
            RoomMat::kTransparent,
            RoomMat::kTransparent,
            RoomMat::kTransparent,
            RoomMat::kTransparent,
            RoomMat::kTransparent,
            RoomMat::kTransparent
        };
        SpinLock wallMatMtx;
        std::atomic<bool> wallsNeedUpdate = false;

        RoomData();
        ~RoomData();
    private:
        vraudio::ResonanceAudioApi* audioEngine = nullptr;
        UnorderedMap<entity_t, vraudio::ResonanceAudioApi::SourceId> sourceMap;
        SingleAudioRenderBuffer workingBuffers;

        void ConsiderAudioSource(const PlanarSampleBufferInlineView& monoSourceData, const vector3& worldPos, const quaternion& worldRot, const matrix4& invRoomTransform, entity_t ownerID, const vector3& roomHalfExts);

        void RenderSpace(PlanarSampleBufferInlineView& outBuffer, PlanarSampleBufferInlineView& scratchBuffer, const vector3& listenerPosRoomSpace, const quaternion& listenerRotRoomSpace, const vector3& roomHalfExts, const RoomProperties& roomProperties);

        void DeleteAudioDataForEntity(entity_t entity);
    };
    
    Ref<RoomData> roomData;
    vector3 roomHalfExts{ 10, 10, 10 };
    RoomProperties roomProperties;
public:
    BoxReverbationAudioSpace(Entity owner);
    void DebugDraw(RavEngine::DebugDrawer& dbg, const RavEngine::Transform& tr) const override {}
    auto GetData() const {
        return roomData;
    }
    auto GetHalfExts() const {
        return roomHalfExts;
    }

    void SetHalfExts(const decltype(roomHalfExts)& h) {
        roomHalfExts = h;
        roomData->wallsNeedUpdate = true;
    }

    void SetWallMaterial(uint8_t idx, RoomMat material) {
        std::lock_guard guard(roomData->wallMatMtx);
        roomData->wallMaterials.at(idx) = material;
        roomData->wallsNeedUpdate = true;
    }
    void SetWallMaterials(const Array<RoomMat, 6>& materials) {
        std::lock_guard guard(roomData->wallMatMtx);
        roomData->wallMaterials = materials;
        roomData->wallsNeedUpdate = true;
    }
    // note: returns a copy!!
    auto GetWallMaterials() const {
        std::lock_guard guard(roomData->wallMatMtx);
        return roomData->wallMaterials;
    }
    const auto& GetRoomProperties() const {
        return roomProperties;
    }
    auto& GetRoomProperties() {
        return roomProperties;
    }
};


}
#endif
