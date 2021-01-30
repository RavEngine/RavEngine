#pragma once
#include <api/resonance_audio_api.h>
#include "mathtypes.hpp"
#include "Ref.hpp"
#include "ComponentStore.hpp"
#include "SpinLock.hpp"
#include "Queryable.hpp"

namespace RavEngine{

/**
 Renders audio buffers based on its owning world's state
 */
class AudioRoom : public Component, public Queryable<AudioRoom>{
friend class AudioSyncSystem;
public:
	static constexpr uint16_t NFRAMES = 32;
protected:
	vraudio::ResonanceAudioApi* audioEngine = nullptr;
	vraudio::ResonanceAudioApi::SourceId src = vraudio::ResonanceAudioApi::kInvalidSourceId;
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

};
}
