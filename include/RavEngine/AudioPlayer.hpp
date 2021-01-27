#pragma once
#include <SDL_audio.h>
#include "Ref.hpp"

namespace RavEngine{
/**
 Is responsible for making the buffers generated in the Audio Engine class come out your speakers
 */
class World;
class AudioPlayer{
	SDL_AudioDeviceID device;
public:
	/**
	 Set the current world to output audio for
	 */
	void SetWorld(Ref<World> w);
	
	/**
	 Initialize the audio player
	 */
	void Init();
	
	/**
	 Shut down the audio player
	 */
	void Shutdown();
	};

}
