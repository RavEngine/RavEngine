#include "AudioPlayer.hpp"
#if !RVE_SERVER
#include <SDL.h>
#include <phonon.h>
#endif
#include "Debug.hpp"
#include "World.hpp"
#include "AudioSource.hpp"
#include "AudioSpace.hpp"
#include "DataStructures.hpp"
#include "AudioGraphAsset.hpp"
#include "App.hpp"
#include <algorithm>
#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

using namespace RavEngine;
using namespace std;


STATIC(AudioPlayer::SamplesPerSec) = 0;
STATIC(AudioPlayer::nchannels) = 0;
STATIC(AudioPlayer::buffer_size) = 0;


struct AudioWorker : public tf::WorkerInterface{
    void scheduler_prologue(tf::Worker& worker) final{
#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
        pthread_setname_np(
            #if __linux__
                    pthread_self(),
            #endif
                    "Audio Worker"
                    );
#endif
    }
    void scheduler_epilogue(tf::Worker& worker, std::exception_ptr ptr) final{};
};

#if !RVE_SERVER
std::string_view IPLerrorToString(IPLerror error){
    switch(error){
        case IPL_STATUS_SUCCESS:    return "IPL_STATUS_SUCCESS";
        IPL_STATUS_FAILURE:         return "IPL_STATUS_FAILURE";
        IPL_STATUS_OUTOFMEMORY:     return "IPL_STATUS_OUTOFMEMORY";
        IPL_STATUS_INITIALIZATION:  return "IPL_STATUS_INITIALIZATION";
        default:
            return "Invalid IPLerror";
    }
}

AudioPlayer::AudioPlayer() : audioExecutor{2, std::make_shared<AudioWorker>()}{
    
}

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

    audioExecutor.run(audioTaskflow).wait();
    

    TZero(stream, len); // fill with silence
  
    static_assert(sizeof(SnapshotToRender) == sizeof(void*), "Not a pointer! Check this!");

    const auto buffers_size = len / sizeof(float);
    stackarray(shared_buffer, float, buffers_size);
    PlanarSampleBufferInlineView sharedBufferView(shared_buffer, buffers_size, buffers_size / GetNChannels());
    float* accum_buffer = reinterpret_cast<float*>(stream);
    InterleavedSampleBufferView accumView{ accum_buffer,buffers_size };

   
    const auto blendBufferIn = [accumView](PlanarSampleBufferInlineView sourceView){
        const auto nchannels = sourceView.GetNChannels();
#pragma omp simd
        for (int i = 0; i < accumView.size(); i++) {
            //mix with existing
            // also perform planar-to-interleaved conversion
            accumView[i] += sourceView[i % nchannels][i / nchannels];
        }
    };
    
    // copy the mix created by worker threads to the current mix 

    auto view = playerRenderBuffer->GetDataBufferView();

    blendBufferIn(view);
    
    //clipping: clamp all values to [-1,1]
#pragma omp simd
    for (int i = 0; i < accumView.size(); i++) {
        accumView[i] = std::clamp(accumView[i], -1.0f, 1.0f);
    }
    
    globalSamples += sharedBufferView.GetNumSamples();
}

void AudioPlayer::SetupAudioTaskGraph(){
    
    
    auto doPlayer = [](auto renderData, auto player){    // must be a AudioDataProvider. We use Auto here to avoid vtable.
        auto sharedBufferView = renderData->GetDataBufferView();
        auto effectScratchBuffer = renderData->GetScratchBufferView();

        player->ProvideBufferData(sharedBufferView, effectScratchBuffer);   
    };

    auto updateIterators = audioTaskflow.emplace([this] {
        dataProvidersBegin = SnapshotToRender->dataProviders.begin();
        dataProvidersEnd = SnapshotToRender->dataProviders.end();
        ambientSourcesBegin = SnapshotToRender->ambientSources.begin();
        ambientSourcesEnd = SnapshotToRender->ambientSources.end();

        simpleSpacesBegin = SnapshotToRender->simpleAudioSpaces.begin();
        simpleSpacesEnd = SnapshotToRender->simpleAudioSpaces.end();


        //use the first audio listener (TODO: will cause unpredictable behavior if there are multiple listeners)
        lpos = SnapshotToRender->listenerPos;
        lrot = SnapshotToRender->listenerRot;
        matrix4 invListenerTransform = glm::inverse(glm::translate(matrix4(1), (vector3)lpos) * glm::toMat4((quaternion)lrot));
  
        auto lockedworld = SnapshotToRender->sourceWorld.lock();
        if (lockedworld == nullptr) {
            return;
        }
        // copy out the destroyed sources
        destroyedSources.clear();
        {
            entity_t id = INVALID_ENTITY;
            while (lockedworld->destroyedAudioSources.try_dequeue(id)) {
                destroyedSources.push_back(id);
            }
        }

    }).name("Audio Preamble");

    // point sources
    auto processDataProviders = audioTaskflow.for_each(std::ref(dataProvidersBegin), std::ref(dataProvidersEnd), [doPlayer,this](auto&& player) {
        auto renderData = &player->renderData;
        static_assert(sizeof(player) == sizeof(std::shared_ptr<void>), "Not a pointer, check this!");
        doPlayer(renderData, player);
    }).name("Process point source providers").succeed(updateIterators);

    // ambient sources
    auto processAmbients = audioTaskflow.for_each(std::ref(ambientSourcesBegin), std::ref(ambientSourcesEnd), [doPlayer, this](auto&& source) {
        auto renderData = &source->renderData;
        static_assert(sizeof(source) == sizeof(std::shared_ptr<void>), "Not a pointer, check this!");
        doPlayer(renderData, source);
    }).name("Process ambient audio").succeed(updateIterators);

    // once point sources have completed, start processing Rooms
    auto processSimpleRooms = audioTaskflow.for_each(std::ref(simpleSpacesBegin), std::ref(simpleSpacesEnd), [this](auto&& r) {
        auto& room = r.room;

        // destroyed-sources
        for (const auto id : destroyedSources) {
            room->DeleteAudioDataForEntity(id);
        }

        // existing sources
        // first check that the listener is inside the room
        if (!r.IsInsideSourceArea(lpos)) {
            return;
        }

        // the mix for this room
        auto& buffers = room->renderData;

        auto sharedBufferView = buffers.GetDataBufferView();
        auto effectScratchBuffer = buffers.GetScratchBufferView();
        const auto bufferSize = sharedBufferView.size();
        stackarray(mixTemp, float, bufferSize);
        PlanarSampleBufferInlineView mixTempView{ mixTemp, bufferSize, sharedBufferView.GetNumSamples()};


        TZero(mixTemp, bufferSize);

        for (const auto& source : SnapshotToRender->sources) {
            // is this source inside the space? if not, then don't process it
            if (!r.IsInsideSourceArea(source.worldpos)) {
                continue;
            }

            // add this source into the room
            auto& buffer = source.data->renderData;
            auto view = buffer.GetDataBufferView();

            TZero(sharedBufferView.data(), sharedBufferView.size());
            TZero(effectScratchBuffer.data(), effectScratchBuffer.size());

            room->RenderAudioSource(sharedBufferView, effectScratchBuffer,
                view, source.worldpos, source.ownerID,
                invListenerTransform
            );
            AdditiveBlendSamples(mixTempView, sharedBufferView);
        }
        AdditiveBlendSamples(sharedBufferView, mixTempView);


    }).name("Process Simple Audio Rooms").succeed(processDataProviders);

    // once rooms are done, do the final mix
    auto finalMix = audioTaskflow.emplace([this] {

        auto sharedBufferView = playerRenderBuffer->GetDataBufferView();
        auto effectScratchBuffer = playerRenderBuffer->GetScratchBufferView();

        TZero(sharedBufferView.data(), sharedBufferView.size());

        // ambient sources
        for (auto& source : SnapshotToRender->ambientSources) {
            auto& buffer = source->renderData;
            auto view = buffer.GetDataBufferView();

            // mix it in
            AdditiveBlendSamples(sharedBufferView, view);
            
        }

        // rooms
        for (auto& room : SnapshotToRender->simpleAudioSpaces) {
            auto& buffer = room.room->renderData;
            auto view = buffer.GetDataBufferView();

            // mix it in
            AdditiveBlendSamples(sharedBufferView, view);
        }

        const auto bufferSize = sharedBufferView.size();
        stackarray(mixTemp, float, bufferSize);
        PlanarSampleBufferInlineView mixTempView{ mixTemp, bufferSize, bufferSize };

        // run the graph on the listener, if present
        if (SnapshotToRender->listenerGraph) {
            SnapshotToRender->listenerGraph->Render(mixTempView, effectScratchBuffer, nchannels);
            AdditiveBlendSamples(sharedBufferView, mixTempView);
        }

    }).name("Final audio mix").succeed(processDataProviders, processAmbients);
}

/**
 The audio player tick function. Called every time there is an audio update
 @param udata user data for application
 @param stream buffer to write the data into
 @param len the length of the buffer
 */
void AudioPlayer::TickStatic(void *udata, SDL_AudioStream *stream, int additional_amount, int total_amount){
    if (additional_amount > 0) {
        AudioPlayer* player = static_cast<AudioPlayer*>(udata);
        // this works because we can queue more audio than is requested.
        int bytesToGenerate = player->buffer_size * player->nchannels * sizeof(float);
        auto n_iters = std::ceil((float)additional_amount / bytesToGenerate);
        Uint8 *data = SDL_stack_alloc(Uint8, bytesToGenerate);
        if (data) {
            for(int i = 0; i < n_iters; i++){
                TZero(data, bytesToGenerate);
                player->Tick(data, bytesToGenerate);
                SDL_PutAudioStreamData(stream, data, bytesToGenerate);
            }
            SDL_stack_free(data);
        }
    }
}

void AudioPlayer::Init(){
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0){
		Debug::Fatal("Could not init Audio subsystem: {}",SDL_GetError());
	}
	
	SDL_AudioSpec want;
	
	std::memset(&want, 0, sizeof(want));
	want.freq = config_samplesPerSec;
	want.format = SDL_AUDIO_F32;
	want.channels = config_nchannels;
	
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_OUTPUT, &want, AudioPlayer::TickStatic, this);
	
	if (stream == NULL){
		Debug::Fatal("could not open audio device: {}",SDL_GetError());
	}

    
    SamplesPerSec = want.freq;
    nchannels = want.channels;
    buffer_size = config_buffersize;

    playerRenderBuffer.emplace(buffer_size,nchannels);
	
	Debug::LogTemp("Audio Subsystem initialized");
    
    SetupAudioTaskGraph();

    
    SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(stream));
    
    // steamaudio
    IPLContextSettings contextSettings{
        .version = STEAMAUDIO_VERSION
    };
    IPLerror errorCode = iplContextCreate(&contextSettings, &steamAudioContext);
    if (errorCode){
        Debug::Fatal("Cannot init SteamAudio: {}", IPLerrorToString(errorCode));
    }
    
   
    auto audioSettings = GetSteamAudioSettings();

    // load HRTF
    IPLHRTFSettings hrtfSettings{
        .type = IPL_HRTFTYPE_DEFAULT,
        .volume = 1.0
    };

    errorCode = iplHRTFCreate(steamAudioContext, &audioSettings, &hrtfSettings, &steamAudioHRTF);
    if (errorCode) {
        Debug::Fatal("Cannot load HRTF: {}", IPLerrorToString(errorCode));
    }
    
    // load simulator
    IPLSimulationSettings simulationSettings{
        .flags = IPL_SIMULATIONFLAGS_DIRECT,    // this enables occlusion/transmission simulation
        .sceneType = IPL_SCENETYPE_DEFAULT,
    };
    // see below for examples of how to initialize the remaining fields of this structure

    errorCode = iplSimulatorCreate(steamAudioContext, &simulationSettings, &steamAudioSimulator);
    if (errorCode) {
        Debug::Fatal("Cannot create Steam Audio Simulator: {}", IPLerrorToString(errorCode));
    }
}

void AudioPlayer::Shutdown(){
	SDL_CloseAudioDevice(SDL_GetAudioStreamDevice(stream));
    iplHRTFRelease(&steamAudioHRTF);
    iplSimulatorRelease(&steamAudioSimulator);
    iplContextRelease(&steamAudioContext);
}

IPLAudioSettings AudioPlayer::GetSteamAudioSettings() const
{
    IPLAudioSettings audioSettings{};
    audioSettings.samplingRate = SamplesPerSec;
    audioSettings.frameSize = buffer_size; // the size of audio buffers we intend to process
    return audioSettings;
}
#endif


