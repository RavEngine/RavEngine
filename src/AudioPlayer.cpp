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
STATIC(AudioPlayer::nchannels);

template<typename T>
inline static void TMemset(T* data, T value, size_t nData){
    std::fill(data, data+nData, value);
}

template<typename T>
inline static void TZero(T* data, size_t nData){
    TMemset<T>(data, 0, nData);
}

void AudioPlayer::Tick(Uint8* stream, int len) {

    TZero(stream, len); // fill with silence

    GetApp()->SwapRenderAudioSnapshot();
    auto SnapshotToRender = GetApp()->GetRenderAudioSnapshot();
    static_assert(sizeof(SnapshotToRender) == sizeof(void*), "Not a pointer! Check this!");

    //use the first audio listener (TODO: will cause unpredictable behavior if there are multiple listeners)

    auto& lpos = SnapshotToRender->listenerPos;
    auto& lrot = SnapshotToRender->listenerRot;
    const auto buffers_size = len / sizeof(float);
    stackarray(shared_buffer, float, buffers_size);
    stackarray(effect_scratch_buffer, float, buffers_size);
    PlanarSampleBufferInlineView sharedBufferView(shared_buffer, buffers_size, buffers_size / GetNChannels());
    PlanarSampleBufferInlineView effectScratchBuffer(effect_scratch_buffer, buffers_size, buffers_size / GetNChannels());
    float* accum_buffer = reinterpret_cast<float*>(stream);
    InterleavedSampleBufferView accumView{ accum_buffer,buffers_size };
    TZero(effectScratchBuffer.data(), effectScratchBuffer.size());

    // fill temp buffer with 0s
    auto resetShared = [sharedBufferView]() mutable {
        TZero(sharedBufferView.data(), sharedBufferView.size());
    };

    // add blend temp buffer into output buffer
    const auto blendIn = [accumView, sharedBufferView] {
        const auto nchannels = sharedBufferView.GetNChannels();
#pragma omp simd
        for (int i = 0; i < accumView.size(); i++) {
            //mix with existing
            // also perform planar-to-interleaved conversion
            accumView[i] += sharedBufferView[i % nchannels][i / nchannels];
        }
    };

    //midi sources - inverted logic from Rooms
    // game thread only adds non-null players
    auto num = SnapshotToRender->midiPointPlayers.size();
    for (const auto& midiplayer : SnapshotToRender->midiPointPlayers) {
        resetShared();
        midiplayer->RenderMono(sharedBufferView);

        //TODO: this is not very efficient
        for (const auto& r : SnapshotToRender->rooms) {
            int id = 1;
            for (const auto& midisource : SnapshotToRender->midiPointSources) {
                if (midisource.source.midiPlayer == midiplayer) {
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
        for (const auto& source : SnapshotToRender->sources) {
            // add this source into the room
            room->AddEmitter(source.data.get(), source.worldpos, source.worldrot, r.worldpos, r.worldrot, sharedBufferView.sizeOneChannel(), effectScratchBuffer);
        }

        //now simulate the fire-and-forget audio
        resetShared();

        //simulate in the room
        room->Simulate(sharedBufferView,effectScratchBuffer);
        blendIn();
    }

    for (auto& source : SnapshotToRender->ambientSources) {
        resetShared();
        source->GetSampleRegionAndAdvance(sharedBufferView, effectScratchBuffer);

        // mix it in
        blendIn();
    }

    for (const auto& source : SnapshotToRender->ambientMIDIsources) {
        resetShared();
        source.midiPlayer->RenderMono(sharedBufferView);
        blendIn();
    }

    // run the graph on the listener, if present
    if (SnapshotToRender->listenerGraph) {
        SnapshotToRender->listenerGraph->Render(sharedBufferView, effectScratchBuffer, nchannels);
        blendIn();
    }

    //clipping: clamp all values to [-1,1]
#pragma omp simd
    for (int i = 0; i < accumView.size(); i++) {
        accumView[i] = std::clamp(accumView[i], -1.0f, 1.0f);
    }
}

/**
 The audio player tick function. Called every time there is an audio update
 @param udata user data for application
 @param stream buffer to write the data into
 @param len the length of the buffer
 */
void AudioPlayer::TickStatic(void *udata, Uint8 *stream, int len){
    AudioPlayer* player = static_cast<AudioPlayer*>(udata);
    player->Tick(stream, len);
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
	want.callback = AudioPlayer::TickStatic;
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
    nchannels = have.channels;
	
	if (!silence){
        float* data = new float[have.samples]{0};
		silence = std::make_shared<AudioPlayerData>(std::make_shared<AudioAsset>(InterleavedSampleBufferView(data, have.samples),1));
		silence->SetLoop(true);
	}
	
	Debug::LogTemp("Audio Subsystem initialized");
	SDL_PauseAudioDevice(device,0);	//begin audio playback
}

void AudioPlayer::Shutdown(){
	SDL_CloseAudioDevice(device);
}
