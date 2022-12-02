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

STATIC(AudioPlayer::SamplesPerSec);

template<typename T>
inline static void TMemset(T* data, T value, size_t nData){
    std::fill(data, data+nData, value);
}

template<typename T>
inline static void TZero(T* data, size_t nData){
    TMemset<T>(data, 0, nData);
}

/**
 The audio player tick function. Called every time there is an audio update
 @param udata user data for application
 @param stream buffer to write the data into
 @param len the length of the buffer
 */
void AudioPlayer::Tick(void *udata, Uint8 *stream, int len){
    AudioPlayer* player = static_cast<AudioPlayer*>(udata);
    
    TZero(stream, len); // fill with silence
    
    GetApp()->SwapRenderAudioSnapshot();
    auto SnapshotToRender = GetApp()->GetRenderAudioSnapshot();     static_assert(sizeof(SnapshotToRender) == sizeof(void*), "Not a pointer! Check this!");
    
    //use the first audio listener (TODO: will cause unpredictable behavior if there are multiple listeners)
    
    auto& lpos = SnapshotToRender->listenerPos;
    auto& lrot = SnapshotToRender->listenerRot;
    const auto buffers_size = len / sizeof(float);
    stackarray(shared_buffer, float, buffers_size);
    stackarray(effect_scratch_buffer, float, buffers_size);
    InterleavedSampleBuffer sharedBufferView(shared_buffer,buffers_size);
    InterleavedSampleBuffer effectScratchBuffer(effect_scratch_buffer, buffers_size);
    float* accum_buffer = reinterpret_cast<float*>(stream);
    
    // fill temp buffer with 0s
    const auto resetShared = [&shared_buffer, buffers_size]{
        TZero(shared_buffer, buffers_size);
    };
    
    // add blend temp buffer into output buffer
    const auto blendIn = [accum_buffer,&shared_buffer, buffers_size]{
#pragma omp simd
        for (int i = 0; i < buffers_size; i++) {
            //mix with existing
            accum_buffer[i] += shared_buffer[i];
        }
    };
    
    //midi sources - inverted logic from Rooms
    // game thread only adds non-null players
    auto num = SnapshotToRender->midiPointPlayers.size();
    for(const auto& midiplayer : SnapshotToRender->midiPointPlayers){
        resetShared();
        //TODO: no divide by 2 here, use number of channels in audio player config
        midiplayer->RenderMono(InterleavedSampleBuffer(shared_buffer,buffers_size/2));
        
        //TODO: this is not very efficient
        for (const auto& r : SnapshotToRender->rooms) {
            int id = 1;
            for(const auto& midisource: SnapshotToRender->midiPointSources){
                if (midisource.source.midiPlayer == midiplayer){
                    r.room->AddEmitter(shared_buffer, midisource.worldpos, midisource.worldrot, r.worldpos, r.worldrot, midisource.hashcode() * id, midisource.source.midiPlayer->GetVolume());
                    id++;
                }
            }
        }
    }

    for (const auto& r : SnapshotToRender->rooms) {
        auto& room = r.room;
        room->SetListenerTransform(lpos, lrot);
        resetShared();
        
        // raster sources
        for(const auto& source : SnapshotToRender->sources){
            // add this source into the room
            room->AddEmitter(source.data.get(), source.worldpos, source.worldrot, r.worldpos, r.worldrot, len, effectScratchBuffer);
        }
        
        //now simulate the fire-and-forget audio
        resetShared();
        
        //simulate in the room
        room->Simulate(sharedBufferView);
        blendIn();
    }
    
    for (auto& source : SnapshotToRender->ambientSources) {
        resetShared();
        source->GetSampleRegionAndAdvance(sharedBufferView, effectScratchBuffer);
        
        // mix it in
        blendIn();
    }
    
    for (const auto& source : SnapshotToRender->ambientMIDIsources){
        resetShared();
        source.midiPlayer->RenderMono(InterleavedSampleBuffer(shared_buffer,buffers_size));
        blendIn();
    }

    //clipping: clamp all values to [-1,1]
#pragma omp simd
    for(int i = 0; i < buffers_size; i++){
        accum_buffer[i] = std::clamp(accum_buffer[i] ,-1.0f,1.0f);
    }
}


void AudioPlayer::Init(){
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0){
		Debug::Fatal("Could not init Audio subsystem: {}",SDL_GetError());
	}
	
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
    
    SamplesPerSec = have.freq;
    
	
	if (!silence){
        float* data = new float[have.samples]{0};
		silence = std::make_shared<AudioPlayerData>(std::make_shared<AudioAsset>(data, have.samples,1));
		silence->SetLoop(true);
	}
	
	Debug::LogTemp("Audio Subsystem initialized");
	SDL_PauseAudioDevice(device,0);	//begin audio playback
}

void AudioPlayer::Shutdown(){
	SDL_CloseAudioDevice(device);
}
