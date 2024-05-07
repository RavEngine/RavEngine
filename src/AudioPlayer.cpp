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
STATIC(AudioPlayer::maxAudioSampleLatency) = 0;

#define USE_MT_IMPL 0

constexpr auto doPlayer = [](auto renderData, auto player) {    // must be a AudioDataProvider. We use Auto here to avoid vtable.
    auto sharedBufferView = renderData->GetWritableDataBufferView();
    auto effectScratchBuffer = renderData->GetWritableScratchBufferView();

    player->ProvideBufferData(sharedBufferView, effectScratchBuffer);
};

constexpr auto doDataProvider = [](auto&& player) {
    auto renderData = &player->renderData;
    static_assert(sizeof(player) == sizeof(std::shared_ptr<void>), "Not a pointer, check this!");
    doPlayer(renderData, player);
};


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

void AudioPlayer::Tick() {
    
    static_assert(sizeof(SnapshotToRender) == sizeof(void*), "Not a pointer! Check this!");
   
    auto queuedSize = SDL_GetAudioStreamQueued(stream);
    if (queuedSize < maxAudioSampleLatency) {
        GetApp()->SwapRenderAudioSnapshot();
        SnapshotToRender = GetApp()->GetRenderAudioSnapshot();
#if USE_MT_IMPL
        audioExecutor.run(audioTaskflow).wait();
#else
        ST_DoMix();
#endif
        
        SDL_PutAudioStreamData(stream, interleavedOutputBuffer.data(), interleavedOutputBuffer.size() * sizeof(interleavedOutputBuffer[0]));
    }
}

void RavEngine::AudioPlayer::CalculateGeometryAudioSpace(AudioSnapshot::GeometryAudioSpaceData& r) {
    auto& room = r.room;

    // destroyed-sources
    for (const auto id : destroyedSources) {
        room->DeleteAudioDataForEntity(id);
    }

    //TODO: destroyed-meshoccluders

    // first check that the listener is inside the room
    if (!r.IsInsideMeshArea(lpos)) {
        return;
    }

    auto outputView = room->workingBuffers.GetWritableDataBufferView();
    auto outputScratchView = room->workingBuffers.GetWritableScratchBufferView();
    auto accumulationView = room->accumulationBuffer.GetWritableDataBufferView();

    TZero(accumulationView.data(), accumulationView.size());

    // add meshes
    for (const auto& mesh : SnapshotToRender->audioMeshes) {
        room->ConsiderMesh(mesh.asset, mesh.worldTransform, r.worldpos, r.invRoomTransform, mesh.ownerID);
    }

    for (const auto& source : SnapshotToRender->sources) {

        room->ConsiderAudioSource(source.worldpos, source.ownerID, r.worldpos, r.invRoomTransform);
    }

    //room->CalculateRoom(r.invRoomTransform, )
}

void RavEngine::AudioPlayer::CalculateSimpleAudioSpace(AudioSnapshot::SimpleAudioSpaceData& r)
{
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

    auto outputView = room->workingBuffers.GetWritableDataBufferView();
    auto outputScratchView = room->workingBuffers.GetWritableScratchBufferView();
    auto accumulationView = room->accumulationBuffer.GetWritableDataBufferView();

    TZero(accumulationView.data(), accumulationView.size());

    for (const auto& source : SnapshotToRender->sources) {
        // is this source inside the space? if not, then don't process it
        if (!r.IsInsideSourceArea(source.worldpos)) {
            continue;
        }

        // add this source into the room
        auto& buffer = source.data->renderData;
        auto sourceView = buffer.GetReadonlyDataBufferView();

        TZero(outputView.data(), outputView.size());
        TZero(outputScratchView.data(), outputScratchView.size());

        room->RenderAudioSource(outputView, outputScratchView,
            sourceView, source.worldpos, source.ownerID,
            invListenerTransform
        );
        AdditiveBlendSamples(accumulationView, outputView);
    }

}

void RavEngine::AudioPlayer::CalculateFinalMix()
{
    auto sharedBufferView = playerRenderBuffer->GetWritableDataBufferView();
    auto effectScratchBuffer = playerRenderBuffer->GetWritableScratchBufferView();

    TZero(sharedBufferView.data(), sharedBufferView.size());
    TZero(effectScratchBuffer.data(), effectScratchBuffer.size());

    // ambient sources
    for (auto& source : SnapshotToRender->ambientSources) {
        auto view = source->renderData.GetReadonlyDataBufferView();

        // mix it in
        AdditiveBlendSamples(sharedBufferView, view);

    }

    // rooms
    for (const auto& r : SnapshotToRender->simpleAudioSpaces) {
        const auto view = r.room->accumulationBuffer.GetReadonlyDataBufferView();

#if ENABLE_RINGBUFFERS
        r.room->GetRingBuffer().WriteSampleData(view);
#endif

        // mix it in
        AdditiveBlendSamples(sharedBufferView, view);
    }
    /*
            const auto bufferSize = sharedBufferView.size();
            stackarray(mixTemp, float, bufferSize);
            TZero(mixTemp, bufferSize);
            PlanarSampleBufferInlineView mixTempView{ mixTemp, bufferSize, bufferSize };

            // run the graph on the listener, if present
            if (SnapshotToRender->listenerGraph) {
                SnapshotToRender->listenerGraph->Render(mixTempView, effectScratchBuffer, nchannels);
                AdditiveBlendSamples(sharedBufferView, mixTempView);
            }
            */

    InterleavedSampleBufferView accumView{ interleavedOutputBuffer.data(), interleavedOutputBuffer.size() };
    TZero(accumView.data(), accumView.size());  // reset to silence

    const auto blendBufferIn = [accumView](PlanarSampleBufferInlineView sourceView) {
        const auto nchannels = sourceView.GetNChannels();
#pragma omp simd
        for (int i = 0; i < accumView.size(); i++) {
            //mix with existing
            // also perform planar-to-interleaved conversion
            accumView[i] += sourceView[i % nchannels][i / nchannels];
        }
        };

    // copy the mix created by worker threads to the current mix 
    blendBufferIn(sharedBufferView);

    //clipping: clamp all values to [-1,1]
#pragma omp simd
    for (int i = 0; i < accumView.size(); i++) {
        accumView[i] = std::clamp(accumView[i], -1.0f, 1.0f);
    }

    globalSamples += GetBufferSize();
}

void RavEngine::AudioPlayer::ST_DoMix()
{
    PerformAudioTickPreamble();

    for (auto& provider : SnapshotToRender->dataProviders) {
        doDataProvider(provider);
    }
    for (auto& ambient : SnapshotToRender->ambientSources) {
        doDataProvider(ambient);
    }

    for (auto& space : SnapshotToRender->simpleAudioSpaces) {
        CalculateSimpleAudioSpace(space);
    }

    for (auto& space : SnapshotToRender->geometryAudioSpaces) {
        CalculateGeometryAudioSpace(space);
    }

    CalculateFinalMix();
}

void RavEngine::AudioPlayer::PerformAudioTickPreamble()
{
    dataProvidersBegin = SnapshotToRender->dataProviders.begin();
    dataProvidersEnd = SnapshotToRender->dataProviders.end();
    ambientSourcesBegin = SnapshotToRender->ambientSources.begin();
    ambientSourcesEnd = SnapshotToRender->ambientSources.end();

    simpleSpacesBegin = SnapshotToRender->simpleAudioSpaces.begin();
    simpleSpacesEnd = SnapshotToRender->simpleAudioSpaces.end();

    geometrySpacesBegin = SnapshotToRender->geometryAudioSpaces.begin();
    geometrySpacesEnd = SnapshotToRender->geometryAudioSpaces.end();


    //use the first audio listener (TODO: will cause unpredictable behavior if there are multiple listeners)
    lpos = SnapshotToRender->listenerPos;
    lrot = SnapshotToRender->listenerRot;
    invListenerTransform = glm::inverse(glm::translate(matrix4(1), (vector3)lpos) * glm::toMat4((quaternion)lrot));

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
}

void AudioPlayer::SetupAudioTaskGraph(){
    

    auto updateIterators = audioTaskflow.emplace([this] {
        PerformAudioTickPreamble();
    }).name("Audio Preamble");

    // point sources
    auto processDataProviders = audioTaskflow.for_each(std::ref(dataProvidersBegin), std::ref(dataProvidersEnd), doDataProvider).name("Process point source providers").succeed(updateIterators);

    // ambient sources
    auto processAmbients = audioTaskflow.for_each(std::ref(ambientSourcesBegin), std::ref(ambientSourcesEnd), doDataProvider).name("Process ambient audio").succeed(updateIterators);

    // once point sources have completed, start processing Rooms
    auto processSimpleRooms = audioTaskflow.for_each(std::ref(simpleSpacesBegin), std::ref(simpleSpacesEnd), [this](AudioSnapshot::SimpleAudioSpaceData& r) {
        CalculateSimpleAudioSpace(r);
    }).name("Process Simple Audio Rooms").succeed(processDataProviders);

    auto processGeometryRooms = audioTaskflow.for_each(std::ref(geometrySpacesBegin), std::ref(geometrySpacesEnd), [this](AudioSnapshot::GeometryAudioSpaceData& r) {
        CalculateGeometryAudioSpace(r);
    }).name("Process Geometry Audio Rooms").succeed(processDataProviders);

    // once rooms are done, do the final mix
    auto finalMix = audioTaskflow.emplace([this] {

        CalculateFinalMix();

    }).name("Final audio mix").succeed(processDataProviders, processAmbients, processSimpleRooms, processGeometryRooms);
    audioTaskflow.dump(std::cout);
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
	
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_OUTPUT, &want, NULL, this);
	
	if (stream == NULL){
		Debug::Fatal("could not open audio device: {}",SDL_GetError());
	}

    
    SamplesPerSec = want.freq;
    nchannels = want.channels;
    buffer_size = config_buffersize;

    playerRenderBuffer.emplace(buffer_size,nchannels);
    interleavedOutputBuffer.resize(buffer_size * nchannels, 0);
    maxAudioSampleLatency = buffer_size * 16;
	
	Debug::LogTemp("Audio Subsystem initialized");
    
#if USE_MT_IMPL
    SetupAudioTaskGraph();
#endif

    
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
    
   

    audioTickThread.emplace([this] {
        while (audioThreadShouldRun) {
            Tick();
        }
    });
    audioTickThread->detach();
}

void AudioPlayer::Shutdown(){
    audioThreadShouldRun = false;
    if (audioTickThread->joinable()) {
        audioTickThread->join();
    }
    audioTickThread.reset();

	SDL_CloseAudioDevice(SDL_GetAudioStreamDevice(stream));
    iplHRTFRelease(&steamAudioHRTF);
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