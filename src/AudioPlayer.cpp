#include "AudioPlayer.hpp"
#include <SDL.h>
#include "Debug.hpp"
#include "World.hpp"
#include "AudioSource.hpp"
#include "AudioRoom.hpp"
#include "DataStructures.hpp"

using namespace RavEngine;
using namespace std;

static WeakRef<World> worldToRender;

/**
 The audio player tick function. Called every time there is an audio update
 @param udata user data for application
 @param stream buffer to write the data into
 @param len the length of the buffer
 */
void AudioPlayer::Tick(void *udata, Uint8 *stream, int len){
	std::memset(stream,0,len);		//fill with silence
	Ref<World> world = worldToRender.lock();
	if (world){
		auto sources = world->GetAllComponentsOfTypeFastPath<AudioSourceComponent>();
		auto rooms = world->GetAllComponentsOfTypeFastPath<AudioRoom>();
		
		//use the first audio listener (TODO: will cause unpredictable behavior if there are multiple listeners)
		if (world->HasComponentOfType<AudioListener>()) {
			auto listener = world->GetComponent<AudioListener>();
			auto listenerTransform = listener->getOwner().lock()->transform();
			auto lpos = listenerTransform->GetWorldPosition();
			auto lrot = listenerTransform->GetWorldRotation();

			stackarray(shared_buffer, float, len / sizeof(float));
			stackarray(accum_buffer, float, len / sizeof(float));

			std::memset(accum_buffer, 0, len);

			for (const auto& r : rooms) {
				Ref<AudioRoom> room = static_pointer_cast<AudioRoom>(r);
				room->SetListenerTransform(lpos, lrot);
				std::memset(shared_buffer, 0, len);
				//simulate in the room
				room->Simulate(shared_buffer, len, sources);
				for (int i = 0; i < len / sizeof(float); i++) {
					//mix with existing
					accum_buffer[i] += shared_buffer[i];
				}
				
				//now simulate the fire-and-forget audio
				std::memset(shared_buffer, 0, len);
				for(auto& f : world->instantaneousToPlay){
					room->SimulateSingle(shared_buffer, len, &f, f.source_position, quaternion(1.0, 0.0, 0.0, 0.0));
				}
				
				//mix again
				for (int i = 0; i < len / sizeof(float); i++) {
					accum_buffer[i] += shared_buffer[i];
				}
			}
			
			//remove sounds from that list that have finished playing
			world->instantaneousToPlay.remove_if([](const InstantaneousAudioSource& ias){
				return ! ias.IsPlaying();
			});

			//update stream pointer with rendered output
			std::memcpy(stream, accum_buffer, len);

			//TODO: mix music (non-spatialized) audio
		}
	}
}


void AudioPlayer::Init(){
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	
	SDL_AudioSpec want, have;
	
	std::memset(&want, 0, sizeof(want));
	want.freq = 44100;
	want.format = AUDIO_F32;
	want.channels = 2;
	want.samples = AudioRoom::NFRAMES;
	want.callback = AudioPlayer::Tick;
	
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
