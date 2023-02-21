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
	friend class AudioSourceBase;
private:
	const float* audiodata;
	double lengthSeconds = 0;
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
	AudioAsset(InterleavedSampleBufferView interleavedData, decltype(nchannels) nchannels) : nchannels(nchannels){
		audiodata = interleavedData.data();
		data = PlanarSampleBufferInlineView{const_cast<float*>(audiodata),interleavedData.size(),nchannels };
		data.ImportInterleavedData(interleavedData, nchannels);
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
        return data.sizeOneChannel();
    }
};

/**
 A render buffer for audio processing. Allocated and managed internally.
 */
struct AudioRenderBuffer{
    struct SingleRenderBuffer{
        std::atomic<uint64_t> lastCompletedProcessingIterationID = 0;
        float* data_impl = nullptr;
        float* scratch_impl = nullptr;
        uint8_t nchannels = 0;
        SingleRenderBuffer(uint16_t nsamples, uint8_t nchannels) : nchannels(nchannels){
            data_impl = new float[nsamples * nchannels]{0};
            scratch_impl = new float[nsamples * nchannels]{0};
        }
        ~SingleRenderBuffer(){
            if (data_impl){
                delete[] data_impl;
            }
            if (scratch_impl){
                delete[] scratch_impl;
            }
        }
        SingleRenderBuffer(SingleRenderBuffer&& other) : nchannels(other.nchannels), data_impl(other.data_impl), scratch_impl(other.scratch_impl){
            lastCompletedProcessingIterationID.store(other.lastCompletedProcessingIterationID.load());
            other.data_impl = nullptr;
            other.scratch_impl = nullptr;
        }
        PlanarSampleBufferInlineView GetDataBufferView() const;
        PlanarSampleBufferInlineView GetScratchBufferView() const;
    };
    std::vector<SingleRenderBuffer> buffers;
    AudioRenderBuffer(uint16_t nBuffers, uint16_t nsamples, uint8_t nchannels){
        buffers.reserve(nBuffers);
        for(decltype(nBuffers) i = 0; i < nBuffers; i++){
            buffers.emplace_back(nsamples, nchannels);
        }
    }
};


struct AudioDataProvider{
    virtual void ProvideBufferData(PlanarSampleBufferInlineView& out_buffer, PlanarSampleBufferInlineView& effectScratchBuffer) = 0;
    
    AudioDataProvider(uint16_t nbuffers, uint16_t nsamples, uint8_t nchannels) : renderData(nbuffers, nsamples, nchannels){}
    
    AudioRenderBuffer renderData;
    float volume = 1;
    bool loops : 1 = false;
    bool isPlaying : 1 = false;
    
    /**
     Starts playing the audio source if it is not playing. Call Pause() to suspend it.
     */
    inline void Play(){
        isPlaying = true;
    }
    
    /**
     Stop the source if it is playing. Call Play() to resume.
     */
    inline void Pause(){
        isPlaying = false;
    }
    
    virtual void Restart() = 0;
    
    
    inline float GetVolume() const { return volume; }
    
    /**
     Change the volume for this source
     @param vol new volume for this source.
     */
    inline void SetVolume(float vol){volume = vol;}
    
    /**
     Enable or disable looping for this audio source. A looping source will continuously play until manually stopped, whereas
     non-looping sources will automatically deactivate when finished
     @param loop new loop setting
     */
    inline void SetLoop(bool loop) {this->loops = loop;}
    
    /**
     @return true if the source is currently playing, false otherwise
     */
    inline bool IsPlaying() const { return isPlaying; }
};


/**
 This is a marker component to indicate where the "microphone" is in the world. Do not have more than one in a world.
 Applying an effect graph to the listener will apply the graph to all sounds in the world at once.
 */
class AudioListener : public Queryable<AudioListener>, public AudioGraphComposed, public AutoCTTI{};

/**
 Player for AudioAssets
 */
struct SampledAudioDataProvider : public AudioGraphComposed, public AudioDataProvider{
    Ref<AudioAsset> asset;
    
    size_t playhead_pos = 0;
    
    SampledAudioDataProvider(decltype(asset) a, uint8_t nchannels = 1);
    
    /**
    * Change the audio asset in this player
    * @param a the audio asset
    */
    inline void SetAudio(decltype(SampledAudioDataProvider::asset) a) {
        asset = a;
    }
    
    /**
     Reset the audio playhead to the beginning of this source. This does not trigger it to begin playing.
     */
    void Restart() final{
        playhead_pos = 0;
    }
    
    /**
     Get the next region, accounting for looping and volume, of the current track. The playhead advances buffer.size() % (loops? numsamples : 1).
     If the next region is shorter than the remaining space in the buffer, that space is filled with 0.
     @param buffer output destination
     */
    void ProvideBufferData(PlanarSampleBufferInlineView& buffer, PlanarSampleBufferInlineView& scratchSpace) final;
};

/**
 Represents a single audio source.
 */
struct AudioSourceBase {
   
protected:
	friend class AudioEngine;
	friend class AudioSyncSystem;
    Ref<AudioDataProvider> player;
	
public:
    
	AudioSourceBase(decltype(player) a) : player(a){}

    inline void SetPlayer(decltype(player) p ){
        player = p;
    }
    
    inline decltype(player) GetPlayer() const{
        return player;
    }

};

/**
 For attaching a movable source to an Entity. Affected by Rooms.
 */
struct AudioSourceComponent : public AudioSourceBase, public Queryable<AudioSourceComponent>, public AutoCTTI{
    AudioSourceComponent(Ref<AudioDataProvider> a);
};

/**
 For playing omnipresent audio in a scene. Not affected by Rooms.
 */
struct AmbientAudioSourceComponent : public AudioSourceBase, public Queryable< AmbientAudioSourceComponent>, public AutoCTTI {
    AmbientAudioSourceComponent(Ref<AudioDataProvider> a);
};

/**
 Used for Fire-and-forget audio playing. Affected by Rooms. See method on the world for more info
 */
struct InstantaneousAudioSource : public AudioSourceBase{
	vector3 source_position;
	
    InstantaneousAudioSource(Ref<AudioAsset> a, const vector3& position, float vol = 1);
};

/**
 Used for Fire-and-forget audio playing, where spatialization is not necessary. See method on the world for more info
 */
struct InstantaneousAmbientAudioSource : public AudioSourceBase {
    InstantaneousAmbientAudioSource(Ref<AudioAsset> a, float vol = 1);
};

}
