#if !RVE_SERVER

#include "AudioRoom.hpp"
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

SimpleAudioSpace::RoomData::RoomData() {}

void RavEngine::SimpleAudioSpace::RoomData::RenderAudioSource(PlanarSampleBufferInlineView& buffer, PlanarSampleBufferInlineView& scratchBuffer, PlanarSampleBufferInlineView monoSourceData, const vector3& sourcePos, entity_t owningEntity, const matrix4& invListenerTransform)
{
    //TODO: if the source is too far away, bail

    // get the binaural effect
    IPLBinauralEffect binauralEffect;
    auto it = steamAudioData.find(owningEntity);
    if (it == steamAudioData.end()) {
        IPLBinauralEffectSettings effectSettings{};
        auto& audioPlayer = GetApp()->GetAudioPlayer();
        auto state = audioPlayer->GetSteamAudioState();
        effectSettings.hrtf = state.hrtf;

        auto settings = audioPlayer->GetSteamAudioSettings();

        iplBinauralEffectCreate(state.context, &settings, &effectSettings, &binauralEffect);
        steamAudioData.emplace(owningEntity, binauralEffect);
    }
    else {
        binauralEffect = it->second;
    }

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
        .numChannels = 2,
        .numSamples = IPLint32(buffer.GetNumSamples()),
        .data = outputChannels
    };

    auto sourcePosInListenerSpace = vector3(invListenerTransform * vector4(sourcePos,1));
    sourcePosInListenerSpace = glm::normalize(sourcePosInListenerSpace);

    IPLBinauralEffectParams params{
        .direction = { sourcePosInListenerSpace.x,sourcePosInListenerSpace.y,sourcePosInListenerSpace.z },
        .interpolation = IPL_HRTFINTERPOLATION_BILINEAR,
        .spatialBlend = 1.0f,
        .hrtf = GetApp()->GetAudioPlayer()->GetSteamAudioHRTF(),
        .peakDelays = nullptr
    };

    auto result = iplBinauralEffectApply(binauralEffect, &params, &inBuffer, &outputBuffer);

    const auto nchannels = AudioPlayer::GetNChannels();
    AudioGraphComposed::Render(buffer, scratchBuffer, nchannels); // process graph for spatialized audio

}

void RavEngine::SimpleAudioSpace::RoomData::DeleteAudioDataForEntity(entity_t entity)
{
    steamAudioData.erase(entity);
}

//void RavEngine::AudioRoom::DebugDraw(RavEngine::DebugDrawer& dbg, const RavEngine::Transform& tr) const
//{
//	dbg.DrawRectangularPrism(tr.CalculateWorldMatrix(), debug_color, data->roomDimensions);
//}
#endif
