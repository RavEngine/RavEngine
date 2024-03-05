#pragma once
#if !RVE_SERVER
#include <SDL_audio.h>
#endif
#include "Ref.hpp"
#include "WeakRef.hpp"
#include <taskflow/taskflow.hpp>
#include <taskflow/core/worker.hpp>
#include "DataStructures.hpp"

namespace RavEngine{

class World;
struct AudioSourceBase;
struct AudioSnapshot;
struct AudioDataProvider;
/**
 Is responsible for making the buffers generated in the Audio Engine class come out your speakers
 */
class AudioPlayer{
#if !RVE_SERVER
    SDL_AudioStream* stream;
	WeakRef<World> worldToRender;
    uint64_t currentProcessingID = 0;
    uint64_t globalSamples = 0; // in units of a single channel
#endif

    
    static uint32_t SamplesPerSec;
    static uint8_t nchannels;
    static uint16_t buffer_size;
    
    static constexpr uint16_t config_buffersize = 512;
    static constexpr uint16_t config_samplesPerSec = 44'100;
    static constexpr uint32_t config_nchannels = 2;
    static constexpr uint32_t config_nbuffers = 2;  //NOTE: do not change this value, the executor does not currently wait properly for the previous tick to complete before scheduling the next batch
#if !RVE_SERVER
	void Tick(Uint8*, int);

    
    tf::Executor audioExecutor;
    tf::Taskflow audioTaskflow;
    
    AudioSnapshot* SnapshotToRender = nullptr;
    ConcurrentQueue<tf::Future<void>> theFutures;
    
    void EnqueueAudioTasks();
    UnorderedSet<Ref<AudioDataProvider>> alreadyTicked;
    
#endif

    
public:
#if !RVE_SERVER
    AudioPlayer();
    
	/**
	 Set the current world to output audio for
	 */
	inline void SetWorld(Ref<World> w){
		worldToRender = w;
	}

	
	/**
	 Initialize the audio player
	 */
	void Init();
	
	/**
	 Shut down the audio player
	 */
	void Shutdown();
    
    const static auto GetSamplesPerSec() {
        return SamplesPerSec;
    }
    
    const static auto GetNChannels(){
        return nchannels;
    }
    
    const static auto GetBufferSize(){
        return buffer_size;
    }
	
    const static auto GetBufferCount(){
        return config_nbuffers;
    }
    
    decltype(globalSamples) GetGlobalAudioTime() const{
        return globalSamples;
    }
#endif
    
	/**
	 Tick function, used internally
	 */
#if !RVE_SERVER
	static void TickStatic(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount);
#endif
};

}
