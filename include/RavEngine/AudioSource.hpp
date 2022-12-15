#pragma once
#include "Queryable.hpp"
#include <string>
#include "mathtypes.hpp"
#include "Ref.hpp"
#include "Debug.hpp"
#include "AudioTypes.hpp"

namespace RavEngine{

class AudioAsset{
	friend class AudioEngine;
	friend class AudioSyncSystem;
	friend class AudioPlayerData;
private:
	const float* audiodata;
	double lengthSeconds = 0;
	size_t numsamplesOneChannel = 0;      // number of samples in one channel, not whole data size
	uint8_t nchannels = 0;
public:
    PlanarSampleBufferInlineView data;
	/**
	 Construct an AudioAsset given a file path. The AudioAsset will decode the audio into samples.
	 @param name the file name to load
	 @param desired_channels the number of channels the file should have after loading
	 */
	AudioAsset(const std::string& name, decltype(nchannels) desired_channels = 1);
	
	/**
	 Use for generated audio. The AudioAsset assumes ownership of the data and will free it on destruction.
	 @param data the audio sample data, in [-1,1] normalized float
	 @param n_samples the number of samples in the buffer. This number is not sanity-checked.
	 @param nchannels the number of channels in the buffer. This number is not sanity-checked.
	 */
	AudioAsset(const float* data, size_t n_samples, decltype(nchannels) nchannels) : numsamplesOneChannel(n_samples), nchannels(nchannels), audiodata(data){
        //TODO: fix
        Debug::Fatal("TODO - does not handle planar audio");
    }
	
	~AudioAsset();
	
	inline double GetLength() const {return lengthSeconds;}
	inline decltype(nchannels) GetNChanels() const {
		return nchannels;
	}
    
    inline auto GetData() const{
        return audiodata;
    }
    
    inline auto GetNumSamples() const{
        return numsamplesOneChannel;
    }
};


/**
 This is a marker component to indicate where the "microphone" is in the world. Do not have more than one in a world.
 Applying an effect graph to the listener will apply the graph to all sounds in the world at once.
 */
class AudioListener : public Queryable<AudioListener>, public AudioGraphComposed, public AutoCTTI{};

/**
 Represents a single audio source.
 */
struct AudioPlayerData {
    struct Player : public AudioGraphComposed{
        Ref<AudioAsset> asset;
        float volume = 1;
        size_t playhead_pos = 0;
        bool loops : 1;
        bool isPlaying : 1;
        Player(decltype(asset) a) : asset(a), loops(false), isPlaying(false){}
        
        /**
         Get the next region, accounting for looping and volume, of the current track. The playhead advances buffer.size() % (loops? numsamples : 1).
         If the next region is shorter than the remaining space in the buffer, that space is filled with 0.
         @param buffer output destination
         */
        inline void GetSampleRegionAndAdvance(PlanarSampleBufferInlineView& buffer, PlanarSampleBufferInlineView& scratchSpace){
            assert(buffer.GetNChannels() == asset->nchannels);  // you are trying to do something that doesn't make sense!!
            for(size_t i = 0; i < buffer.sizeOneChannel(); i++){
                //is playhead past end of source?
                if (playhead_pos >= asset->numsamplesOneChannel){
                    if (loops){
                        playhead_pos = 0;
                    }
                    else{
#pragma omp simd
                        for(uint8_t c = 0; c < asset->nchannels; c++){
                            buffer[c][i] = 0;
                        }
                        isPlaying = false;
                        continue;
                    }
                }
#pragma omp simd
                for(uint8_t c = 0; c < asset->nchannels; c++){
                    buffer[c][i] = asset->data[c][playhead_pos] * volume;
                }
                playhead_pos++;
            }
            AudioGraphComposed::Render(buffer,scratchSpace, asset->GetNChanels());
        }
    };
protected:
	friend class AudioEngine;
	friend class AudioSyncSystem;
    Ref<Player> player;
	
public:
    
    void SetGraph(AudioGraphComposed::effect_graph_ptr_t inGraph){
        player->SetGraph(inGraph);
    }
    
    auto GetGraph() const{
        return player->GetGraph();
    }
    
	AudioPlayerData(decltype(Player::asset) a ) : player(std::make_shared<Player>(a)){}

    inline decltype(player) GetPlayer() const{
        return player;
    }
    
	/**
	* Change the audio asset in this player
	* @param a the audio asset
	*/
    inline void SetAudio(decltype(Player::asset) a) {
		player->asset = a;
	}
	
	/**
	 Starts playing the audio source if it is not playing. Call Pause() to suspend it.
	 */
    inline void Play(){
		player->isPlaying = true;
	}
	
	/**
	 Stop the source if it is playing. Call Play() to resume.
	 */
    inline void Pause(){
		player->isPlaying = false;
	}
	
	/**
	 Reset the audio playhead to the beginning of this source. This does not trigger it to begin playing.
	 */
    inline void Restart(){
		player->playhead_pos = 0;
	}
	
    inline float GetVolume() const { return player->volume; }
	
	/**
	 Change the volume for this source
	 @param vol new volume for this source.
	 */
    inline void SetVolume(float vol){player->volume = vol;}
	
	/**
	 Enable or disable looping for this audio source. A looping source will continuously play until manually stopped, whereas
	 non-looping sources will automatically deactivate when finished
	 @param loop new loop setting
	 */
    inline void SetLoop(bool loop) {player->loops = loop;}
	
	/**
	 @return true if the source is currently playing, false otherwise
	 */
    inline bool IsPlaying() const { return player->isPlaying; }

};

/**
 For attaching a movable source to an Entity. Affected by Rooms.
 */
struct AudioSourceComponent : public AudioPlayerData, public Queryable<AudioSourceComponent>, public AutoCTTI{
	AudioSourceComponent(Ref<AudioAsset> a) : AudioPlayerData(a){
		if (a->GetNChanels() != 1) {
			Debug::Fatal("Only mono is supported for point-audio sources, got {} channels", a->GetNChanels());
		}
	}
};

/**
 For playing omnipresent audio in a scene. Not affected by Rooms.
 */
struct AmbientAudioSourceComponent : public AudioPlayerData, public Queryable< AmbientAudioSourceComponent>, public AutoCTTI {
	AmbientAudioSourceComponent(Ref<AudioAsset> a) : AudioPlayerData(a) {}
};

/**
 Used for Fire-and-forget audio playing. Affected by Rooms. See method on the world for more info
 */
struct InstantaneousAudioSource : public AudioPlayerData{
	vector3 source_position;
	
	InstantaneousAudioSource(Ref<AudioAsset> a, const vector3& position, float vol = 1) : AudioPlayerData(a), source_position(position){
		player->volume = vol;
		player->isPlaying = true;
	}
};

/**
 Used for Fire-and-forget audio playing, where spatialization is not necessary. See method on the world for more info
 */
struct InstantaneousAmbientAudioSource : public AudioPlayerData {
	InstantaneousAmbientAudioSource(Ref<AudioAsset> a, float vol = 1) : AudioPlayerData(a) {
		player->volume = vol;
		player->isPlaying = true;
	}
};

}
