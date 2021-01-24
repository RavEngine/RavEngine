#pragma once
#include <api/resonance_audio_api.h>
#include "mathtypes.hpp"
#include "Ref.hpp"

namespace RavEngine{

class Entity;

/**
 Renders audio buffers based on its owning world's state
 */
class AudioEngine{
friend class AudioSyncSystem;
protected:
	vraudio::ResonanceAudioApi* audioEngine = nullptr;
public:
	static constexpr uint16_t NFRAMES = 32;
	
	AudioEngine(){
		audioEngine = vraudio::CreateResonanceAudioApi(2, NFRAMES, 44100);
	}
	~AudioEngine(){
		delete audioEngine;
	}
	
	/**
	 Update the position of the listener in the Audio Engine
	 @param worldpos the position of the listener in world space
	 @param worldrotation the rotation of the listener in world space
	 */
	void SetListenerTransform(const vector3& worldpos, const quaternion& worldrotation);
	
	/**
	 Make the audio engine aware of the entity and its sound components
	 */
	void Spawn(Ref<Entity>);
	
	/**
	 Make the audio engine no longer aware of the entity and its sound components
	 */
	void Destroy(Ref<Entity>);
	
	/**
	 Generate an audio buffer
	 */
	void Tick(float fpsScale);
};

/**
 Is responsible for making the buffers generated in the above class come out your speakers
 */
class AudioPlayer{
	
};

}
