#include "AudioPlayer.hpp"
#include <SDL.h>
#include "Debug.hpp"
#include "World.hpp"
#include "AudioSource.hpp"
#include "AudioRoom.hpp"
#include "DataStructures.hpp"
#include "App.hpp"
#include <algorithm>

using namespace RavEngine;
using namespace std;

Ref<AudioPlayerData> AudioPlayer::silence;

/**
 The audio player tick function. Called every time there is an audio update
 @param udata user data for application
 @param stream buffer to write the data into
 @param len the length of the buffer
 */
void AudioPlayer::Tick(void *udata, Uint8 *stream, int len){
	AudioPlayer* player = static_cast<AudioPlayer*>(udata);
	
	std::memset(stream,0,len);		//fill with silence
    App::SwapRenderAudioSnapshot();
    auto SnapshotToRender = App::GetRenderAudioSnapshot();
    auto& sources = SnapshotToRender->sources;
    //auto& rooms = world->GetAllComponentsOfType<AudioRoom>();
    auto& ambientSources = SnapshotToRender->ambientSources;
		
    //use the first audio listener (TODO: will cause unpredictable behavior if there are multiple listeners)
    
    //TODO: FIX
    auto& lpos = SnapshotToRender->listenerPos;
    auto& lrot = SnapshotToRender->listenerRot;
    stackarray(shared_buffer, float, len / sizeof(float));
    stackarray(accum_buffer, float, len / sizeof(float));
    std::memset(accum_buffer, 0, len);
//
//            if (rooms){
//                for (const auto& r : *rooms.value()) {
//                    Ref<AudioRoom> room = static_pointer_cast<AudioRoom>(r);
//                    room->SetListenerTransform(lpos, lrot);
//                    std::memset(shared_buffer, 0, len);
//                    if (sources){
//                        for(const auto& source :* sources.value()){
//                            // add this source into the room
//                            if (auto owner = source.GetOwner().lock()){
//                                auto tr = owner->GetTransform();
//                                auto ptr = static_pointer_cast<AudioSourceComponent>(source);
//                                room->AddEmitter(ptr.get(), tr->GetWorldPosition(), tr->GetWorldRotation(), len);
//                            }
//                        }
//                    }
//
//
//                    //now simulate the fire-and-forget audio
//                    std::memset(shared_buffer, 0, len);
//                    for(auto& f : world->instantaneousToPlay){
//                        room->AddEmitter(&f, f.source_position, quaternion(1.0, 0.0, 0.0, 0.0), len);
//                    }
//
//
//                    //simulate in the room
//                    room->Simulate(shared_buffer, len);
//                    for (int i = 0; i < len / sizeof(float); i++) {
//                        //mix with existing
//                        accum_buffer[i] += shared_buffer[i];
//                    }
//                }
//
//            }
//

    for (auto& source : ambientSources) {

        source->GetSampleRegionAndAdvance(shared_buffer, len);

        // mix it in
        for (int i = 0; i < len / sizeof(float); i++) {
            accum_buffer[i] += shared_buffer[i];
        }
    }

    //clipping: clamp all values to [-1,1]
    for(int i = 0; i < len/sizeof(float); i++){
        accum_buffer[i] = std::clamp(accum_buffer[i] ,-1.0f,1.0f);
    }

    //update stream pointer with rendered output
    std::memcpy(stream, accum_buffer, len);
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
	want.userdata = this;
	
	device = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
	
	if (device == 0){
		Debug::Fatal("could not open audio device: {}",SDL_GetError());
	}
	else{
		if (have.format != want.format){
			Debug::Fatal("Could not get Float32 audio format");
		}
	}
	
	if (!silence){
		float* data = new float[4096];
		std::memset(data, 0, sizeof(float) * 4096);
		silence = std::make_shared<AudioPlayerData>(std::make_shared<AudioAsset>(data, 4096,1));
		silence->SetLoop(true);
	}
	
	Debug::LogTemp("Audio Subsystem initialized");
	SDL_PauseAudioDevice(device,0);	//begin audio playback
}

void AudioPlayer::Shutdown(){
	SDL_CloseAudioDevice(device);
}
