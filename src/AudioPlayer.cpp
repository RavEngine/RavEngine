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

STATIC(AudioPlayer::SamplesPerSec) = 0;
STATIC(AudioPlayer::nchannels) = 0;
STATIC(AudioPlayer::buffer_size) = 0;

template<typename T>
inline static void TMemset(T* data, T value, size_t nData){
    std::fill(data, data+nData, value);
}

template<typename T>
inline static void TZero(T* data, size_t nData){
    TMemset<T>(data, 0, nData);
}

void AudioPlayer::Tick(Uint8* stream, int len) {
    
    GetApp()->SwapRenderAudioSnapshot();
    SnapshotToRender = GetApp()->GetRenderAudioSnapshot();
    auto buffer_idx = currentProcessingID % GetBufferCount();
        
    // kill all the remaining tasks
    // if there are any, because that means they missed the deadline
    tf::Future<void> future;
    while (theFutures.try_dequeue(future)){
        future.cancel();
    }
    
    audioExecutor.run(audioTaskflow);   // does not wait - get ahead on process ID +1
    TZero(stream, len); // fill with silence
  
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

    const auto blendBufferIn = [accumView](PlanarSampleBufferInlineView sourceView){
        const auto nchannels = sourceView.GetNChannels();
#pragma omp simd
        for (int i = 0; i < accumView.size(); i++) {
            //mix with existing
            // also perform planar-to-interleaved conversion
            accumView[i] += sourceView[i % nchannels][i / nchannels];
        }
    };
    
    // add blend temp buffer into output buffer
    const auto blendIn = [&blendBufferIn,accumView, sharedBufferView] {
        blendBufferIn(sharedBufferView);
    };
    
  

    //midi sources - inverted logic from Rooms
    // game thread only adds non-null players
    auto num = SnapshotToRender->midiPointPlayers.size();
    for (const auto& midiplayer : SnapshotToRender->midiPointPlayers) {
        // does this player have up-to-date samples? if so, include it, otherwise skip it
        auto& buffer = midiplayer->renderData.buffers[buffer_idx];
        auto proc_id = buffer.lastCompletedProcessingIterationID.load();
        if (proc_id == currentProcessingID){
            resetShared();
            auto bufferview = buffer.GetDataBufferView();
            
            //TODO: this is not very efficient - but does it matter?
            for (const auto& r : SnapshotToRender->rooms) {
                int id = 1;
                for (const auto& midisource : SnapshotToRender->midiPointSources) {
                    if (midisource.source.midiPlayer == midiplayer) {
                        // because these are mono, buffer.data() is legit
                        r.room->AddEmitter(bufferview.data(), midisource.worldpos, midisource.worldrot, r.worldpos, r.worldrot, midisource.hashcode() * id, midisource.source.midiPlayer->GetVolume());
                        id++;
                    }
                    
                }
            }
        }
        else{
            // miss!
            //cout << fmt::format("miss! {} - {}", proc_id, currentProcessingID) << endl;
        }
    }

    for (const auto& r : SnapshotToRender->rooms) {
        auto& room = r.room;
        room->SetListenerTransform(lpos, lrot);
        resetShared();

        // raster sources
        for (const auto& source : SnapshotToRender->sources) {
            // add this source into the room
            auto& buffer = source.data->renderData.buffers[buffer_idx];
            auto view = buffer.GetDataBufferView();
            auto proc_id = buffer.lastCompletedProcessingIterationID.load();
            if (proc_id == currentProcessingID){
                auto hashcode = std::hash<decltype(source.data.get())>()(source.data.get());
                room->AddEmitter(view.data(), source.worldpos, source.worldrot, r.worldpos, r.worldrot, hashcode, source.data->volume);
            }
        }

        //now simulate the fire-and-forget audio
        resetShared();

        //simulate in the room
        room->Simulate(sharedBufferView,effectScratchBuffer);
        blendIn();
    }

    for (auto& source : SnapshotToRender->ambientSources) {
        auto& buffer = source->renderData.buffers[buffer_idx];
        auto proc_id = buffer.lastCompletedProcessingIterationID.load();
        if (proc_id == currentProcessingID){
            resetShared();

            auto view = buffer.GetDataBufferView();
            
            // mix it in
            blendBufferIn(view);
        }
    }

    // run the graph on the listener, if present
    if (SnapshotToRender->listenerGraph) {
        SnapshotToRender->listenerGraph->Render(sharedBufferView, effectScratchBuffer, nchannels);
        blendIn();
    }
    currentProcessingID++;  // advance proc id to mark it as completed
    //clipping: clamp all values to [-1,1]
#pragma omp simd
    for (int i = 0; i < accumView.size(); i++) {
        accumView[i] = std::clamp(accumView[i], -1.0f, 1.0f);
    }
}

void AudioPlayer::EnqueueAudioTasks(){
    if (!SnapshotToRender){
        return;
    }

    // render out all the buffers
    int i = 1;
    decltype(currentProcessingID) nextID = currentProcessingID + i;   // this is the buffer slot we will render
    auto buffer_idx = nextID % GetBufferCount();
    
    auto doPlayer = [buffer_idx, nextID](auto renderData, auto player){    // must be a AudioDataProvider. We use Auto here to avoid vtable.
        auto& buffers = renderData->buffers[buffer_idx];
        auto sharedBufferView = buffers.GetDataBufferView();
        auto effectScratchBuffer = buffers.GetScratchBufferView();
        
        player->ProvideBufferData(sharedBufferView, effectScratchBuffer);
        buffers.lastCompletedProcessingIterationID = nextID;   // mark it as having completed in this iter cycle
    };
    
    // midi players
    for(const auto& midiplayer : SnapshotToRender->midiPointSources){
        auto renderData = &midiplayer.source.midiPlayer->renderData;
        auto player = midiplayer.source.midiPlayer;
        static_assert(sizeof(player) == sizeof(std::shared_ptr<void>), "Not a pointer, check this!");
        theFutures.enqueue(audioExecutor.async(doPlayer, renderData, player));
    }
    // raster sources
    for (const auto& source : SnapshotToRender->sources) {
        auto renderData = &source.data->renderData;
        auto player = source.data;
        static_assert(sizeof(player) == sizeof(std::shared_ptr<void>), "Not a pointer, check this!");
        theFutures.enqueue(audioExecutor.async(doPlayer, renderData, player));
    }
    for (auto& source : SnapshotToRender->ambientSources) {
        auto renderData = &source->renderData;
        static_assert(sizeof(source) == sizeof(std::shared_ptr<void>), "Not a pointer, check this!");
        theFutures.enqueue(audioExecutor.async(doPlayer, renderData, source));
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
	want.freq = config_samplesPerSec;
	want.format = AUDIO_F32;
	want.channels = config_nchannels;
	want.samples = config_buffersize;
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
    buffer_size = have.samples;
	
	if (!silence){
        float* data = new float[have.samples]{0};
		silence = std::make_shared<AudioPlayerData>(std::make_shared<AudioAsset>(InterleavedSampleBufferView(data, have.samples),1),1);
		silence->SetLoop(true);
	}
	
	Debug::LogTemp("Audio Subsystem initialized");
    
    audioTaskflow.emplace([this](){
        EnqueueAudioTasks();
    });
    
	SDL_PauseAudioDevice(device,0);	//begin audio playback
}

void AudioPlayer::Shutdown(){
	SDL_CloseAudioDevice(device);
}
