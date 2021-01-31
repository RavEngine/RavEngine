#pragma once
#include <api/resonance_audio_api.h>
#include "mathtypes.hpp"
#include "Ref.hpp"
#include "ComponentStore.hpp"
#include "SpinLock.hpp"
#include "Queryable.hpp"
#include "AudioRoomSyncSystem.hpp"
#include "AudioRoomMaterial.hpp"

namespace RavEngine{

/**
 Renders audio buffers based on its owning world's state
 */
class AudioRoom : public Component, public Queryable<AudioRoom>{
	friend class RavEngine::AudioRoomSyncSystem;
public:
	static constexpr uint16_t NFRAMES = 4096;
protected:
	vraudio::ResonanceAudioApi* audioEngine = nullptr;
	vraudio::ResonanceAudioApi::SourceId src = vraudio::ResonanceAudioApi::kInvalidSourceId;

	vector3 roomDimensions;
public:
	
	AudioRoom(){
		audioEngine = vraudio::CreateResonanceAudioApi(2, NFRAMES, 44100);
		src = audioEngine->CreateSoundObjectSource(vraudio::RenderingMode::kBinauralLowQuality);
	}
	~AudioRoom(){
		delete audioEngine;
		src = vraudio::ResonanceAudioApi::kInvalidSourceId;
	}
	
	/**
	 Set the dimensions of the room. A dimension of 0 is interpreted as unbounded.
	 @param dim the x, y, and z dimensions of the room. Unlike Physics, this is diameter not radius.
	 */
	void SetRoomDimensions(const vector3& dim){ roomDimensions = dim; }
	
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
	void Simulate(float* ptr, size_t nbytes, const ComponentStore<SpinLock>::entry_type& sources);
	
	/**
	 @return the dimensions of this room
	 */
	decltype(roomDimensions) GetRoomDimensions() const {return roomDimensions; }
	
	void SetRoomMaterial(const RoomMaterial& properties){
		audioEngine->SetReverbProperties(static_cast<RoomMaterial>(properties));
	}
};

}
