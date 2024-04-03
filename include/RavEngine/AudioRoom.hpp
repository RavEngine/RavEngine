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
        
        float radius = INFINITY;
        
        /**
         Add an emitter for this simulation from arbitrary data
         @param data the data buffer to pass. Must be AudioRoom::NFRAMES in length and represent MONO audio
         @param pos location to play at
         @param rot rotation of the emitter
         @param roompos the worldspace location of the room
         @param roomrot the worldspace rotation of the room
         @param source_hashcode the hashcode for the source emitter. Must be repeatable across calls and should not have collisions.
         @param volume loudness of the source
         */
        void AddEmitter(const float* data, const vector3& pos, const quaternion& rot, const vector3& roompos, const quaternion& roomrot, size_t source_hashcode, float volume);
        
        
        /**
         Update the position of the listener in the Audio Engine
         @param worldpos the position of the listener in world space
         @param worldrotation the rotation of the listener in world space
         */
        void SetListenerTransform(const vector3& worldpos, const quaternion& worldrotation);
        
        /**
         Simulate spacial audio for a set of audio sources
         @param buffer destination for the calculated audio
         */
        void Simulate(PlanarSampleBufferInlineView& buffer, PlanarSampleBufferInlineView& scratchBuffer);

        void RenderAudioSource(
            PlanarSampleBufferInlineView& buffer, PlanarSampleBufferInlineView& scratchBuffer,
            PlanarSampleBufferInlineView monoSourceData, const vector3& sourcePos, entity_t owningEntity,
            const matrix4& invListenerTransform
        );

        // internal use only. Called when an audio source component is destroyed
        void DeleteAudioDataForEntity(entity_t entity);
        
        RoomData();
        ~RoomData(){

        }
    private:
        locked_hashmap<entity_t, _IPLBinauralEffect_t*,SpinLock> steamAudioData;
    };
    Ref<RoomData> data;
	
    
	SimpleAudioSpace(entity_t owner) : ComponentWithOwner(owner), data(std::make_shared<RoomData>()) {}
	
	
	
	/**
	 Render the debug shape for this room. Invoke in a debug rendering component
	 */
    void DebugDraw(RavEngine::DebugDrawer& dbg, const RavEngine::Transform& tr) const override {}
};

}
#endif
