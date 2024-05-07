#pragma once
#if !RVE_SERVER
#include <SDL_audio.h>
#endif
#include "Ref.hpp"
#include "WeakRef.hpp"
#include <taskflow/taskflow.hpp>
#include <taskflow/core/worker.hpp>
#include "DataStructures.hpp"
#include "Types.hpp"
#include "AudioSnapshot.hpp"
#include <semaphore>

struct _IPLContext_t;
struct _IPLHRTF_t;
struct _IPLAudioSettings_t;

namespace RavEngine{

class World;
struct AudioSourceBase;
struct AudioSnapshot;
struct AudioDataProvider;
/**
 Is responsible for making the buffers generated in the Audio Engine class come out your speakers
 */
class AudioPlayer{
#if !RVE_SERVER
    SDL_AudioStream* stream = nullptr;
	WeakRef<World> worldToRender;
    uint64_t globalSamples = 0; // in units of a single channel
    _IPLContext_t* steamAudioContext = nullptr;
    _IPLHRTF_t* steamAudioHRTF = nullptr;
#endif

    
    static uint32_t SamplesPerSec;
    static uint8_t nchannels;
    static uint16_t buffer_size;
    static uint32_t maxAudioSampleLatency;
    
    static constexpr uint16_t config_buffersize = 512;
    static constexpr uint16_t config_samplesPerSec = 44'100;
    static constexpr uint32_t config_nchannels = 2;
#if !RVE_SERVER
	void Tick();

    std::optional<SingleAudioRenderBuffer> playerRenderBuffer;
    
    tf::Executor audioExecutor;
    tf::Taskflow audioTaskflow;
    
    AudioSnapshot* SnapshotToRender = nullptr;
    tf::Future<void> taskflowFuture;
    
    void SetupAudioTaskGraph();
    std::vector<entity_t> destroyedSources, destroyedMeshComponents;

    decltype(AudioSnapshot::dataProviders.begin()) dataProvidersBegin, dataProvidersEnd;
    decltype(AudioSnapshot::ambientSources.begin()) ambientSourcesBegin, ambientSourcesEnd;
    decltype(AudioSnapshot::simpleAudioSpaces.begin()) simpleSpacesBegin, simpleSpacesEnd;
    decltype(AudioSnapshot::geometryAudioSpaces.begin()) geometrySpacesBegin, geometrySpacesEnd;

    vector3 lpos{0};
    quaternion lrot{0,0,0,0};
    matrix4 invListenerTransform{ 1 }, listenerTransform{ 1 };

    std::optional<std::thread> audioTickThread;
    std::atomic<bool> audioThreadShouldRun = true;
    std::vector<float> interleavedOutputBuffer;

    void PerformAudioTickPreamble();
    void CalculateGeometryAudioSpace(AudioSnapshot::GeometryAudioSpaceData& r);
    void CalculateSimpleAudioSpace(AudioSnapshot::SimpleAudioSpaceData&);
    void CalculateFinalMix();
    void ST_DoMix();
#endif

    
public:
#if !RVE_SERVER
    auto GetSteamAudioHRTF() const {
        return steamAudioHRTF;
    }
    auto GetSteamAudioContext() const {
        return steamAudioContext;
    }

    auto GetSteamAudioState() const {
         
        struct SAState{
            decltype(steamAudioHRTF) hrtf;
            decltype(steamAudioContext) context;
        } state{
            .hrtf = steamAudioHRTF,
            .context = steamAudioContext
        };
        return state;
    }
    _IPLAudioSettings_t GetSteamAudioSettings() const;

    AudioPlayer();
    
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
    
    decltype(globalSamples) GetGlobalAudioTime() const{
        return globalSamples;
    }
#endif
    
};

}
