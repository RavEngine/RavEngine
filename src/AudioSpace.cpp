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

#include "mathtypes.hpp"

using namespace RavEngine;
using namespace std;

void RavEngine::SimpleAudioSpace::RoomData::RenderAudioSource(PlanarSampleBufferInlineView& buffer, PlanarSampleBufferInlineView& scratchBuffer, PlanarSampleBufferInlineView monoSourceData, const vector3& sourcePos, entity_t owningEntity, const matrix4& invListenerTransform)
{
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
            .numChannels = 1,
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
    steamAudioData.if_contains(entity, [](SteamAudioEffects& effects) {
        iplBinauralEffectRelease(&effects.binauralEffect);
        iplDirectEffectRelease(&effects.directEffect);
    });
    steamAudioData.erase(entity);
}

RavEngine::SimpleAudioSpace::RoomData::RoomData() : workingBuffers(AudioPlayer::GetBufferSize(), AudioPlayer::GetNChannels()), accumulationBuffer{ AudioPlayer::GetBufferSize(), AudioPlayer::GetNChannels() }
# if ENABLE_RINGBUFFERS
, debugBuffer(AudioPlayer::GetNChannels())
#endif
{
}
# if ENABLE_RINGBUFFERS
void RavEngine::SimpleAudioSpace::RoomData::OutputSampleData(const Filesystem::Path& path) const
{
    debugBuffer.DumpToFileNoProcessing(path);
}
#endif

void RavEngine::GeometryAudioSpace::RoomData::ConsiderAudioSource(PlanarSampleBufferInlineView monoSourceData, const vector3& sourcePos, entity_t owningEntity, const matrix4& invListenerTransform)
{

}

void RavEngine::GeometryAudioSpace::RoomData::RenderSpace(PlanarSampleBufferInlineView& outBuffer, PlanarSampleBufferInlineView& scratchBuffer)
{
}

void RavEngine::GeometryAudioSpace::RoomData::DeleteAudioDataForEntity(entity_t entity) {

}

void RavEngine::GeometryAudioSpace::RoomData::DeleteMeshDataForEntity(entity_t entity)
{
}

//void RavEngine::AudioRoom::DebugDraw(RavEngine::DebugDrawer& dbg, const RavEngine::Transform& tr) const
//{
//	dbg.DrawRectangularPrism(tr.CalculateWorldMatrix(), debug_color, data->roomDimensions);
//}
#endif
