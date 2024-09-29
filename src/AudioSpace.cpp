#if !RVE_SERVER

#include "AudioSpace.hpp"
#include "Entity.hpp"
#include "AudioSource.hpp"
#include "DataStructures.hpp"
#include "Transform.hpp"
#include "AudioPlayer.hpp"
#include <phonon.h>
#include "App.hpp"
#include "Debug.hpp"
#include "AudioMeshAsset.hpp"
#include "Profile.hpp"

#include "mathtypes.hpp"
#include <glm/gtc/type_ptr.hpp>

using namespace RavEngine;
using namespace std;

void RavEngine::SimpleAudioSpace::RoomData::RenderAudioSource(PlanarSampleBufferInlineView& buffer, PlanarSampleBufferInlineView& scratchBuffer, PlanarSampleBufferInlineView monoSourceData, const vector3& sourcePos, entity_t owningEntity, const matrix4& invListenerTransform)
{
    RVE_PROFILE_FN;
    // get the binaural effect
    auto& audioPlayer = GetApp()->GetAudioPlayer();
    auto state = audioPlayer->GetSteamAudioState();

    SteamAudioEffects effects;
    auto it = steamAudioData.find(owningEntity);
    if (it == steamAudioData.end()) {
        IPLBinauralEffectSettings effectSettings{};
        effectSettings.hrtf = state.hrtf;

        auto settings = audioPlayer->GetSteamAudioSettings();

        iplBinauralEffectCreate(state.context, &settings, &effectSettings, &effects.binauralEffect);

        IPLDirectEffectSettings directEffectSettings{
            .numChannels = AudioPlayer::GetNChannels(),
        };
        iplDirectEffectCreate(state.context, &settings, &directEffectSettings, &effects.directEffect);

        steamAudioData.emplace(owningEntity, effects);
    }
    else {
        effects = it->second;
    }
    const auto nchannels = AudioPlayer::GetNChannels();

    // render it
    IPLfloat32* inputChannels[]{ monoSourceData.data() };
    static_assert(std::size(inputChannels) == 1, "Input must be mono!");
    IPLAudioBuffer inBuffer{
        .numChannels = 1,
        .numSamples = IPLint32(monoSourceData.GetNumSamples()),
        .data = inputChannels,
    };

    Debug::Assert(buffer.GetNChannels() == 2, "Non-stereo output is not supported");

    IPLfloat32* outputChannels[]{
        buffer[0].data(),
        buffer[1].data()
    };
    IPLAudioBuffer outputBuffer{
        .numChannels = nchannels,
        .numSamples = IPLint32(buffer.GetNumSamples()),
        .data = outputChannels
    };

    auto sourcePosInListenerSpace = vector3(invListenerTransform * vector4(sourcePos,1));
    auto normalizedPos = glm::normalize(sourcePosInListenerSpace);

    IPLBinauralEffectParams params{
        .direction = { normalizedPos.x,normalizedPos.y,normalizedPos.z },
        .interpolation = IPL_HRTFINTERPOLATION_BILINEAR,
        .spatialBlend = 1.0f,
        .hrtf = GetApp()->GetAudioPlayer()->GetSteamAudioHRTF(),
        .peakDelays = nullptr
    };

    auto result = iplBinauralEffectApply(effects.binauralEffect, &params, &inBuffer, &outputBuffer);

    // do distance attenuation in-place
    IPLDistanceAttenuationModel distanceAttenuationModel{
       .type = IPL_DISTANCEATTENUATIONTYPE_DEFAULT
    };
    IPLDirectEffectParams directParams{
        .flags = IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION,
        .distanceAttenuation = iplDistanceAttenuationCalculate(state.context,{sourcePosInListenerSpace.x,sourcePosInListenerSpace.y,sourcePosInListenerSpace.z},{0,0,0},&distanceAttenuationModel)
    };
   
    result = iplDirectEffectApply(effects.directEffect, &directParams, &outputBuffer, &outputBuffer);

    AudioGraphComposed::Render(buffer, scratchBuffer, nchannels); // process graph for spatialized audio

}

void RavEngine::SimpleAudioSpace::RoomData::DeleteAudioDataForEntity(entity_t entity)
{
    steamAudioData.if_contains(entity, [this](SteamAudioEffects& effects) {
        DestroyEffects(effects);
    });
    steamAudioData.erase(entity);
}

RavEngine::SimpleAudioSpace::RoomData::RoomData() : workingBuffers(AudioPlayer::GetBufferSize(), AudioPlayer::GetNChannels()), accumulationBuffer{ AudioPlayer::GetBufferSize(), AudioPlayer::GetNChannels() }
# if ENABLE_RINGBUFFERS
, debugBuffer(AudioPlayer::GetNChannels())
#endif
{
}
RavEngine::SimpleAudioSpace::RoomData::~RoomData() {
    for (auto& [entity, effects] : steamAudioData) {
        DestroyEffects(effects);
    }
}


# if ENABLE_RINGBUFFERS
void RavEngine::SimpleAudioSpace::RoomData::OutputSampleData(const Filesystem::Path& path) const
{
    debugBuffer.DumpToFileNoProcessing(path);
}

#endif
void RavEngine::SimpleAudioSpace::RoomData::DestroyEffects(SteamAudioEffects& effects)
{
    iplBinauralEffectRelease(&effects.binauralEffect);
    iplDirectEffectRelease(&effects.directEffect);
}

RavEngine::GeometryAudioSpace::RoomData::RoomData() : workingBuffers(AudioPlayer::GetBufferSize(), AudioPlayer::GetNChannels()), accumulationBuffer{ AudioPlayer::GetBufferSize(), AudioPlayer::GetNChannels() }{
    // load simulator
    IPLSimulationSettings simulationSettings{
        .flags = IPLSimulationFlags(IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_PATHING),    // this enables occlusion/transmission simulation
        .sceneType = IPL_SCENETYPE_DEFAULT,
        .reflectionType = IPL_REFLECTIONEFFECTTYPE_CONVOLUTION, // disabled if flags does not include IPL_SIMULATIONFLAGS_REFLECTIONS
        .maxNumRays = 4096,
        .numDiffuseSamples = 32,
        .maxDuration = 2.0f,
        .maxOrder = 1,
        .maxNumSources = 8,
        .numThreads = 2,
        .samplingRate = IPLint32(AudioPlayer::GetSamplesPerSec()),
        .frameSize = AudioPlayer::GetBufferSize()
    };
    // see below for examples of how to initialize the remaining fields of this structure

    auto context = GetApp()->GetAudioPlayer()->GetSteamAudioContext();

    auto errorCode = iplSimulatorCreate(context, &simulationSettings, &steamAudioSimulator);
    if (errorCode) {
        Debug::Fatal("Cannot create Steam Audio Simulator: {}", int(errorCode));
    }


    IPLSceneSettings sceneSettings{
        .type = IPL_SCENETYPE_DEFAULT
    };
    iplSceneCreate(context, &sceneSettings, &rootScene);

    iplSimulatorSetScene(steamAudioSimulator, rootScene);

}

RavEngine::GeometryAudioSpace::RoomData::~RoomData()
{
    for (auto& [entity, sourceData] : steamAudioSourceData) {
        DestroySteamAudioSourceConfig(sourceData);
    }
    for (auto& [entity, meshData] : steamAudioMeshData) {
        DestroySteamAudioMeshConfig(meshData);
    }

    iplSimulatorRelease(&steamAudioSimulator);
    iplSceneRelease(&rootScene);
}

constexpr static auto geometrySpaceSimulationFlags = IPLSimulationFlags(IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS | IPL_SIMULATIONFLAGS_PATHING); //TODO: make this configurable


void RavEngine::GeometryAudioSpace::RoomData::ConsiderAudioSource(const vector3& sourcePos, entity_t owningEntity, const vector3& roomPos, const matrix4& invRoomTransform)
{

    // determine if in-radius or not
    bool inRange = glm::distance2(sourcePos, roomPos) <= sourceRadius * sourceRadius;

    // if not in-radius, but previously was in-radius, then destroy associated steam audio data 

    SteamAudioSourceConfig sourceData;
    auto it = steamAudioSourceData.find(owningEntity);
    if (it == steamAudioSourceData.end()) {

        // out of range, and wasn't in range before. Skip
        if (!inRange) {
            return;
        }

        IPLSourceSettings sourceSettings{
            .flags = geometrySpaceSimulationFlags
        };


        iplSourceCreate(steamAudioSimulator, &sourceSettings, &sourceData.source);
        iplSourceAdd(sourceData.source, steamAudioSimulator);

        auto& audioPlayer = GetApp()->GetAudioPlayer();
        auto state = audioPlayer->GetSteamAudioState();
        auto settings = audioPlayer->GetSteamAudioSettings();

        // create effects
        IPLDirectEffectSettings directEffectSettings{
           .numChannels = AudioPlayer::GetNChannels(),
        };
        iplDirectEffectCreate(state.context, &settings, &directEffectSettings, &sourceData.directEffect);

        IPLPathEffectSettings pathEffectSettings{
            .maxOrder = 1,  //TODO: is this a good number? we should make this configurable
            .spatialize = IPL_TRUE,
            .speakerLayout = {
                .type = IPL_SPEAKERLAYOUTTYPE_STEREO,   // other values are optional if type != IPL_SPEAKERLAYOUTTYPE_CUSTOM
            },
            .hrtf = state.hrtf
        };

        iplPathEffectCreate(state.context, &settings, &pathEffectSettings, &sourceData.pathEffect);

        IPLBinauralEffectSettings effectSettings{
            .hrtf = state.hrtf
        };

        iplBinauralEffectCreate(state.context, &settings, &effectSettings, &sourceData.binauralEffect);


        it = steamAudioSourceData.emplace(owningEntity, sourceData).first;
    }
    else {
        if (!inRange) {
            // destroy data and bail
            iplSourceRemove(sourceData.source,steamAudioSimulator);
            DestroySteamAudioSourceConfig(sourceData);
            steamAudioSourceData.erase(owningEntity);
            return;
        }
        else {
            sourceData = it->second;
        }
    }
   
    // set source config
    // all positions are in room space

    auto sourceInRoomSpace = invRoomTransform * vector4(sourcePos,1);

    it->second.roomSpacePos = sourceInRoomSpace;

    IPLSimulationInputs inputs{
        .flags = geometrySpaceSimulationFlags,
        .directFlags = IPLDirectSimulationFlags(IPL_DIRECTSIMULATIONFLAGS_OCCLUSION | IPL_DIRECTSIMULATIONFLAGS_TRANSMISSION),
        .source = {sourceInRoomSpace.x, sourceInRoomSpace.y, sourceInRoomSpace.z},
        .distanceAttenuationModel = IPL_DISTANCEATTENUATIONTYPE_DEFAULT,
        .airAbsorptionModel = IPL_AIRABSORPTIONTYPE_DEFAULT,
        .directivity = {        //TODO: allow setting these on audio sources
            .dipoleWeight = 0,  // purely omni
            .dipolePower = 1,   // direction sharpness
            .callback = nullptr,
            .userData = nullptr,
        },
        .occlusionType = IPL_OCCLUSIONTYPE_RAYCAST,
        .occlusionRadius = 1,   //TODO: what effect does this have? (ignored if occlusion type is not volumetric)
        .numOcclusionSamples = 0,   // ignored if occlusion type is not volumetric
        .reverbScale = {1,1,1},
        .hybridReverbTransitionTime = 1,    //TODO what's a good number for this?
        .hybridReverbOverlapPercent = 0.25, //TODO: what's a good number for this?
        .baked = IPL_FALSE,
        .bakedDataIdentifier = {},  // unused if not baked
        .pathingProbes = nullptr,
        .visRadius = 1, // TODO: what's a good number for this?
        .visThreshold = 0.75,
        .visRange = sourceRadius * 2,   // diameter of the room
        .pathingOrder = 1,
        .enableValidation = IPL_TRUE,
        .findAlternatePaths = IPL_FALSE,    //TODO: is there overhead to using this?
        .numTransmissionRays = 4
    };
    iplSourceSetInputs(sourceData.source, geometrySpaceSimulationFlags, &inputs);
    
}

void RavEngine::GeometryAudioSpace::RoomData::CalculateRoom(const matrix4& invRoomTransform, const vector3& listenerForwardWorldSpace, const vector3& listenerUpWorldSpace, const vector3& listenerRightWorldSpace)
{
    RVE_PROFILE_FN;
    iplSimulatorCommit(steamAudioSimulator);        // apply all queued changes

    const auto forwardRoomSpace = invRoomTransform * vector4(listenerForwardWorldSpace, 1);
    const auto upRoomSpace = invRoomTransform * vector4(listenerUpWorldSpace, 1);
    const auto rightRoomSpace = invRoomTransform * vector4(listenerRightWorldSpace, 1);

    IPLSimulationSharedInputs sharedInputs{
        .listener = {
            .right = {rightRoomSpace.x, rightRoomSpace.y, rightRoomSpace.z},
            .up = {upRoomSpace.x, upRoomSpace.y, upRoomSpace.z},
            .ahead = {forwardRoomSpace.x, forwardRoomSpace.y, forwardRoomSpace.z},
            .origin = {0,0,0}
         },
        .numRays = 32,      // TODO: make these configurable
        .numBounces = 3,
        .duration = 1,
        .order = 1,
        .irradianceMinDistance = 0.01,
        .pathingVisCallback = nullptr,
        .pathingUserData = nullptr,
    };
    iplSimulatorSetSharedInputs(steamAudioSimulator, geometrySpaceSimulationFlags, &sharedInputs);

    //TODO: make this configurable
    iplSimulatorRunDirect(steamAudioSimulator);
    //iplSimulatorRunPathing(steamAudioSimulator);          // TODO: need to setup probes (https://valvesoftware.github.io/steam-audio/doc/capi/guide.html#static-geometry) before using this
    //iplSimulatorRunReflections(steamAudioSimulator);
}

void RavEngine::GeometryAudioSpace::RoomData::ConsiderMesh(Ref<AudioMeshAsset> mesh, const matrix4& transform, const vector3& roomPos, const matrix4& invRoomTransform, entity_t ownerID)
{
    const auto meshPos = transform * vector4(0,0,0,1);

    // is it inside the bounds?
    bool inRange = glm::distance(vector3(meshPos), roomPos) <= mesh->GetRadius() + meshRadius;

    SteamAudioMeshConfig meshConfig;
    auto it = steamAudioMeshData.find(ownerID);
    if (it == steamAudioMeshData.end()) {
        // not in range, and wasn't in range before? bail
        if (!inRange) {
            return;
        }

        auto transformInRoomSpace = glm::transpose(invRoomTransform * transform);   // col-major to row-major

        static_assert(sizeof(transformInRoomSpace) == sizeof(float) * 4 * 4, "transform is not a 4x4 float matrix!");

        // create mesh data
        IPLInstancedMeshSettings meshSettings{
            .subScene = mesh->GetScene(),
        };
        memcpy(meshSettings.transform.elements, glm::value_ptr(transformInRoomSpace), sizeof(transformInRoomSpace));

        iplInstancedMeshCreate(rootScene, &meshSettings, &meshConfig.instancedMesh);
        steamAudioMeshData.emplace(ownerID, meshConfig);
    }
    else {
        if (!inRange) {
            // was in range but no longer is. Destroy its data
            DestroySteamAudioMeshConfig(it->second);
            steamAudioMeshData.erase(ownerID);
            return;
        }

        meshConfig = it->second;
    }

    // set mesh data
    const auto posInRoomSpace = invRoomTransform * meshPos;
    const auto rotInRoomSpace = glm::quat_cast(invRoomTransform * transform);
    if (meshConfig.lastPos != vector3(posInRoomSpace) || meshConfig.lastRot != rotInRoomSpace) {
        meshConfig.lastPos = posInRoomSpace;
        meshConfig.lastRot = rotInRoomSpace;

        auto transformInRoomSpace = glm::transpose(invRoomTransform * transform);   // col-major to row-major

        IPLMatrix4x4 transform;
        memcpy(transform.elements, glm::value_ptr(transformInRoomSpace), sizeof(transformInRoomSpace));

        iplInstancedMeshUpdateTransform(meshConfig.instancedMesh, rootScene, transform);
    }
}

void RavEngine::GeometryAudioSpace::RoomData::RenderAudioSource(PlanarSampleBufferInlineView& outBuffer, PlanarSampleBufferInlineView& scratchBuffer, entity_t sourceOwningEntity, PlanarSampleBufferInlineView monoSourceData, const matrix4& invListenerTransform)
{
    auto it = steamAudioSourceData.find(sourceOwningEntity);
    if (it == steamAudioSourceData.end()) {
        // something invalid has happend!
        Debug::Fatal("Attempting to render source that was not calculated in CalculateRoom()");
    }
    else {
        auto& effects = it->second;

        IPLSimulationOutputs outputs{};

        auto& sourceData = it->second;
        iplSourceGetOutputs(sourceData.source, IPLSimulationFlags(IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_PATHING | IPL_SIMULATIONFLAGS_REFLECTIONS), &outputs);

        IPLfloat32* inputChannels[]{ monoSourceData.data() };
        static_assert(std::size(inputChannels) == 1, "Input must be mono!");
        IPLAudioBuffer inBuffer{
            .numChannels = 1,
            .numSamples = IPLint32(monoSourceData.GetNumSamples()),
            .data = inputChannels,
        };

        const auto nchannels = outBuffer.GetNChannels();
        Debug::Assert(outBuffer.GetNChannels() == 2, "Non-stereo output is not supported");

        IPLfloat32* outputChannels[]{
            outBuffer[0].data(),
            outBuffer[1].data()
        };
        IPLAudioBuffer outputBuffer{
            .numChannels = nchannels,
            .numSamples = IPLint32(outBuffer.GetNumSamples()),
            .data = outputChannels
        };

        //iplPathEffectApply(effects.pathEffect, &outputs.pathing, &inBuffer, &outputBuffer); 

        auto listenerSpaceDir = glm::normalize(invListenerTransform * vector4(sourceData.roomSpacePos,1));
        
        //TODO: replace this with a pathing effect
        IPLBinauralEffectParams params{
              .direction = { listenerSpaceDir.x,listenerSpaceDir.y,listenerSpaceDir.z },
              .interpolation = IPL_HRTFINTERPOLATION_BILINEAR,
              .spatialBlend = 1.0f,
              .hrtf = GetApp()->GetAudioPlayer()->GetSteamAudioHRTF(),
              .peakDelays = nullptr
        };

        auto result = iplBinauralEffectApply(effects.binauralEffect, &params, &inBuffer, &outputBuffer);
        
        iplDirectEffectApply(effects.directEffect, &outputs.direct, &outputBuffer, &outputBuffer);

        AudioGraphComposed::Render(outBuffer, scratchBuffer, nchannels); // process graph for spatialized audio
    }
    
}

void RavEngine::GeometryAudioSpace::RoomData::DeleteAudioDataForEntity(entity_t entity) {
    steamAudioSourceData.if_contains(entity, [this](SteamAudioSourceConfig& effects) {
        DestroySteamAudioSourceConfig(effects);
    });
    steamAudioSourceData.erase(entity);
}

void RavEngine::GeometryAudioSpace::RoomData::DeleteMeshDataForEntity(entity_t entity)
{
    steamAudioMeshData.if_contains(entity, [this](SteamAudioMeshConfig& effects) {
        DestroySteamAudioMeshConfig(effects);
    });
    steamAudioMeshData.erase(entity);
}

void RavEngine::GeometryAudioSpace::RoomData::DestroySteamAudioSourceConfig(SteamAudioSourceConfig& effects)
{
    iplBinauralEffectRelease(&effects.binauralEffect);
    iplDirectEffectRelease(&effects.directEffect);
    iplPathEffectRelease(&effects.pathEffect);
    iplSourceRelease(&effects.source);
}

void RavEngine::GeometryAudioSpace::RoomData::DestroySteamAudioMeshConfig(SteamAudioMeshConfig& config)
{
    iplInstancedMeshRelease(&config.instancedMesh);
}

//void RavEngine::AudioRoom::DebugDraw(RavEngine::DebugDrawer& dbg, const RavEngine::Transform& tr) const
//{
//	dbg.DrawRectangularPrism(tr.CalculateWorldMatrix(), debug_color, data->roomDimensions);
//}
#endif

