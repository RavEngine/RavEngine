#pragma once
#include "mathtypes.hpp"
#include "Ref.hpp"
#include "ComponentWithOwner.hpp"
#include "Queryable.hpp"
#include "AudioRoomMaterial.hpp"
#if !RVE_SERVER

#include "AudioSource.hpp"
#include "Types.hpp"
#include "DebugDrawer.hpp"
#include "AudioRingbuffer.hpp"
#include "Filesystem.hpp"

struct _IPLBinauralEffect_t;
struct _IPLDirectEffect_t;
struct _IPLSource_t;
struct _IPLSimulator_t;
struct _IPLScene_t;

namespace RavEngine{

class AudioRoomSyncSystem;
class AudioPlayer;
struct AudioMeshAsset;

#define ENABLE_RINGBUFFERS 1

/**
 Renders audio buffers based on its owning world's state
 */
class SimpleAudioSpace : public ComponentWithOwner, public IDebugRenderable, public Queryable<SimpleAudioSpace,IDebugRenderable>{
	friend class RavEngine::AudioRoomSyncSystem;
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
	
    
	SimpleAudioSpace(entity_t owner) : ComponentWithOwner(owner), data(std::make_shared<RoomData>()) {}
	
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

struct GeometryAudioSpace : public ComponentWithOwner, public Queryable<GeometryAudioSpace, IDebugRenderable> {
    friend class RavEngine::AudioRoomSyncSystem;
    friend class RavEngine::AudioPlayer;
public:

    struct RoomData {
        RoomData();
        ~RoomData();
        float sourceRadius = 10, meshRadius = 10;


        void ConsiderAudioSource(
            PlanarSampleBufferInlineView monoSourceData, const vector3& sourcePos, entity_t owningEntity,
            const matrix4& invListenerTransform);

        void ConsiderMesh(Ref<AudioMeshAsset> mesh);

        void RenderSpace(
            PlanarSampleBufferInlineView& outBuffer, PlanarSampleBufferInlineView& scratchBuffer
        );

        // internal use only. Called when an audio source component is destroyed
        void DeleteAudioDataForEntity(entity_t entity);
        void DeleteMeshDataForEntity(entity_t entity);
    private:
        struct SteamAudioSourceConfig {
            _IPLSource_t* source = nullptr;
        };

        void DestroySteamAudioSourceConfig(SteamAudioSourceConfig&);


        locked_hashmap<entity_t, SteamAudioSourceConfig, SpinLock> steamAudioSourceData;

        _IPLSimulator_t* steamAudioSimulator = nullptr;
        _IPLScene_t* rootScene = nullptr;
    };

    const auto GetData() const {
        return data;
    }

    GeometryAudioSpace(entity_t owner) : ComponentWithOwner(owner), data(std::make_shared<RoomData>()) {}
private:
    Ref<RoomData> data;
};

}
#endif
