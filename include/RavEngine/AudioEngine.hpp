#pragma once
#include <api/resonance_audio_api.h>

namespace RavEngine{
class AudioEngine{
protected:
	vraudio::ResonanceAudioApi* audioEngine = nullptr;
public:
	AudioEngine(){
		audioEngine = vraudio::CreateResonanceAudioApi(2, 5, 42000);
	}
	~AudioEngine(){
		delete audioEngine;
	}
};
}
