#pragma once
#include <SDL_audio.h>
#include "Ref.hpp"
#include "WeakRef.hpp"

namespace RavEngine{

class World;
struct AudioPlayerData;
/**
 Is responsible for making the buffers generated in the Audio Engine class come out your speakers
 */
class AudioPlayer{
	SDL_AudioDeviceID device;
	WeakRef<World> worldToRender;
	static Ref<AudioPlayerData> silence;
    
    static uint32_t SamplesPerSec;
    static uint8_t nchannels;
    static uint16_t buffer_size;
    
    static constexpr uint16_t config_buffersize = 512;
    static constexpr uint16_t config_samplesPerSec = 44'100;
    static constexpr uint32_t config_nchannels = 2;

	void Tick(Uint8*, int);
    
public:
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
	
	/**
	 Tick function, used internally
	 */
	static void TickStatic(void *udata, Uint8 *stream, int len);
};

}
