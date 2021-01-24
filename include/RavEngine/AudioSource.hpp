#include <api/resonance_audio_api.h>
#include "Component.hpp"
#include "Queryable.hpp"
#include <vector>

namespace RavEngine{

class AudioAsset{
friend class AudioEngine;
private:
	const float* audiodata[1];
public:
	AudioAsset(const std::string& name);
	~AudioAsset();
};

/**
 This is a marker component to indicate where the "microphone" is in the world. Do not have more than one in a world.
 */
class AudioListener : public Component, public Queryable<AudioListener>{};

/**
 Represents a single audio source. To represent multiple sources, simply attach multiple of this component type to your Entity.
 */
class AudioSourceComponent : public Component, public Queryable<AudioSourceComponent>{
protected:
	friend class AudioEngine;
	friend class AudioSyncSystem;
	Ref<AudioAsset> asset;
	float volume = 1;
	double playhead_pos = 0;
	bool loops = false;
	bool isPlaying = false;
	bool playbackSpeed = 1;
	
	vraudio::ResonanceAudioApi::SourceId resonance_handle = vraudio::ResonanceAudioApi::kInvalidSourceId;
public:
	AudioSourceComponent(Ref<AudioAsset> a ) : asset(a){}
	
	inline void Play(){
		isPlaying = true;
	}
	
	inline void Pause(){
		isPlaying = false;
	}
	
	inline void Restart(){
		playhead_pos = 0;
	}
	
	void Tick(float scale);
};

}
