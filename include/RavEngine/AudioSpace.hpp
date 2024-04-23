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

struct _IPLBinauralEffect_t;
struct _IPLDirectEffect_t;

namespace RavEngine{

class AudioRoomSyncSystem;
class AudioPlayer;

/**
 Renders audio buffers based on its owning world's state
 */
class SimpleAudioSpace : public ComponentWithOwner, public IDebugRenderable, public Queryable<SimpleAudioSpace,IDebugRenderable>{
	friend class RavEngine::AudioRoomSyncSystem;
	friend class RavEngine::AudioPlayer;
public:
    struct RoomData : public AudioGraphComposed{
        
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
        
    private:
        struct SteamAudioEffects {
            _IPLBinauralEffect_t* binauralEffect = nullptr;
            _IPLDirectEffect_t* directEffect = nullptr;
        };
        locked_hashmap<entity_t, SteamAudioEffects,SpinLock> steamAudioData;
    };
    Ref<RoomData> data;
	
    
	SimpleAudioSpace(entity_t owner) : ComponentWithOwner(owner), data(std::make_shared<RoomData>()) {}
	
	
	
	/**
	 Render the debug shape for this room. Invoke in a debug rendering component
	 */
    void DebugDraw(RavEngine::DebugDrawer& dbg, const RavEngine::Transform& tr) const override {}
};

struct GeometryAudioSpace : public ComponentWithOwner, public Queryable<GeometryAudioSpace, IDebugRenderable> {
    friend class RavEngine::AudioRoomSyncSystem;
    friend class RavEngine::AudioPlayer;
public:

    struct RoomData {
        float sourceRadius = 10, meshRadius = 10;


        void ConsiderAudioSource(
            PlanarSampleBufferInlineView monoSourceData, const vector3& sourcePos, entity_t owningEntity,
            const matrix4& invListenerTransform);

        void ConsiderMesh();

        void RenderSpace(
            PlanarSampleBufferInlineView& outBuffer, PlanarSampleBufferInlineView& scratchBuffer
        );

        // internal use only. Called when an audio source component is destroyed
        void DeleteAudioDataForEntity(entity_t entity);
        void DeleteMeshDataForEntity(entity_t entity);
    private:

    };

    Ref<RoomData> data;

    GeometryAudioSpace(entity_t owner) : ComponentWithOwner(owner), data(std::make_shared<RoomData>()) {}

};

}
#endif
