#include <api/resonance_audio_api.h>
#include "Component.hpp"
#include "Queryable.hpp"
#include <vector>

namespace RavEngine{

class AudioAsset{
	friend class AudioEngine;
	friend class AudioSyncSystem;
	friend class AudioSourceComponent;
private:
	const float* audiodata;
	double lengthSeconds = 0;
	size_t numsamples = 0;
	size_t frameSize = 0;
public:
	AudioAsset(const std::string& name);
	~AudioAsset();
	
	inline size_t GetFrameSize() const {return frameSize;}
	inline double getLength() const {return lengthSeconds;}
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
	size_t playhead_pos = 0;
	bool loops = false;
	bool isPlaying = false;
	float playbackSpeed = 1;
	
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
	
	inline bool IsPlaying(){ return isPlaying; }

	/**
	 Generate an audio data buffer based on the current source
	 @param buffer destination for the data
	 @param count the size of the buffer, in bytes
	 */
	inline void GetSampleRegionAndAdvance(float* buffer, size_t count){
		// figure out how much overrun
		auto audiosizebytes = asset->numsamples * sizeof(asset->audiodata[0]);
		auto samplesize = sizeof(asset->audiodata[0]);
		
		int overrun = (playhead_pos * samplesize + count) - audiosizebytes;
		if (overrun > 0){
			//if looping, wrap around
			if (loops){
				assert(false);	//TODO: loop does not correctly wrap playhead!
				auto amt = audiosizebytes - playhead_pos * samplesize;
				std::memcpy(buffer, asset->audiodata + playhead_pos, amt);
				playhead_pos += amt / samplesize;
				//recursively repeat
				GetSampleRegionAndAdvance(buffer + amt, count - overrun);
			}
			else{
				//otherwise fill remaining space in the buffer with silence
				std::memcpy(buffer, asset->audiodata + playhead_pos, asset->numsamples - playhead_pos);
				std::memset(buffer + overrun, 0, count - overrun);
				
				playhead_pos = asset->numsamples;
				isPlaying = false;
			}
		}
		else{
			//simply memcpy to fill buffer
			std::memcpy(buffer,asset->audiodata + playhead_pos, count);
			playhead_pos += count / samplesize;
		}
	}
};

}
