#include "AudioPlayer.hpp"
#include <SDL.h>
#include "Debug.hpp"

using namespace RavEngine;

static Ref<World> worldToRender;

/**
 The audio player tick function. Called every time there is an audio update
 @param udata user data for application
 @param stream buffer to write the data into
 @param len the length of the buffer
 */
static void AudioPlayer_Tick(void *udata, Uint8 *stream, int len){
	//TODO: check world pointer
	
	std::memset(stream,0,len);		//fill with silence
	Debug::LogTemp("Buffer {}",len);
}

void AudioPlayer::Init(){
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	
	SDL_AudioSpec want, have;
	
	std::memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
	want.freq = 48000;
	want.format = AUDIO_F32;
	want.channels = 2;
	want.samples = 4096;
	want.callback = AudioPlayer_Tick; /* you wrote this function elsewhere -- see SDL_AudioSpec for details */
	
	device = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
	
	if (device == 0){
		Debug::Fatal("could not open audio device: {}",SDL_GetError());
	}
	else{
		if (have.format != want.format){
			Debug::Fatal("Could not get Float32 audio format");
		}
	}
	Debug::LogTemp("Audio Subsystem initialized");
	SDL_PauseAudioDevice(device,0);	//begin audio playback
}

void AudioPlayer::Shutdown(){
	SDL_CloseAudioDevice(device);
}

void AudioPlayer::SetWorld(Ref<World> w){
	worldToRender = w;
}
