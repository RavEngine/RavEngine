#pragma once
#include <api/resonance_audio_api.h>
#include "mathtypes.hpp"
#include "Ref.hpp"
#include "ComponentStore.hpp"
#include "SpinLock.hpp"
#include "Queryable.hpp"
#include "AudioRoomMaterial.hpp"
#include "AudioSource.hpp"
#include "Entity.hpp"
#include "DebugDrawer.hpp"

namespace RavEngine{

class AudioRoomSyncSystem;
class AudioPlayer;

using RoomMat = vraudio::MaterialName;

/**
 Renders audio buffers based on its owning world's state
 */
class AudioRoom : public Component, public IDebugRenderable, public Queryable<AudioRoom,IDebugRenderable>{
	friend class RavEngine::AudioRoomSyncSystem;
	friend class RavEngine::AudioPlayer;
public:
	static constexpr uint16_t NFRAMES = 4096;
private:
	vraudio::ResonanceAudioApi* audioEngine = nullptr;
    UnorderedMap<size_t,vraudio::ResonanceAudioApi::SourceId> allSources;

	vector3 roomDimensions = vector3(0,0,0);	//size of 0 = infinite
	
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
	
	float reflection_scalar = 1, reverb_gain = 1, reverb_time = 1.0, reverb_brightness = 0;
	
	/**
	 Add an emitter for this simulation
	 @param source the sound to play
	 @param pos location to play at
	 @param rot rotation of emitter
	 @param nbytes number of bytes to get from the source
	 */
	void AddEmitter(AudioPlayerData* source, const vector3& pos, const quaternion& rot, size_t nbytes);
	

	void DestroyEmitterFor(AudioPlayerData* source);
	
public:
	
	AudioRoom(){
		audioEngine = vraudio::CreateResonanceAudioApi(2, NFRAMES, 44100);
	}
	~AudioRoom(){
		delete audioEngine;
	}
	
	/**
	 Set the dimensions of the room. A dimension of 0 is interpreted as unbounded.
	 @param dim the x, y, and z dimensions of the room. Unlike Physics, this is diameter not radius.
	 */
	constexpr void SetRoomDimensions(const vector3& dim){ roomDimensions = dim; }
	
	/**
	 Update the position of the listener in the Audio Engine
	 @param worldpos the position of the listener in world space
	 @param worldrotation the rotation of the listener in world space
	 */
    void SetListenerTransform(const vector3& worldpos, const quaternion& worldrotation);
	
	/**
	 Simulate spacial audio for a set of audio sources
	 @param ptr destination for the calculated audio
	 @param nbytes length of the buffer in bytes
	 @param sources the AudioSource components to calculate for
	 */
    void Simulate(float* ptr, size_t nbytes);
	
	/**
	 @return the dimensions of this room
	 */
	constexpr inline decltype(roomDimensions) GetRoomDimensions() const {return roomDimensions; }
	
	/**
	 @return a writable reference to the wall materials
	 */
    constexpr inline decltype(wallMaterials)& WallMaterials() { return wallMaterials; }
	
	/**
	 Render the debug shape for this room. Invoke in a debug rendering component
	 */
    void DebugDraw(DebugDrawer& dbg) const final;
};

}
