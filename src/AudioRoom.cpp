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

void SimpleAudioSpace::RoomData::SetListenerTransform(const vector3 &worldpos, const quaternion &wr){
#if 0
	audioEngine->SetHeadPosition(worldpos.x, worldpos.y, worldpos.z);
	audioEngine->SetHeadRotation(wr.x, wr.y, wr.z, wr.w);
#endif
}

void SimpleAudioSpace::RoomData::AddEmitter(const float* data, const vector3 &pos, const quaternion &rot, const vector3 &roompos, const quaternion &roomrot, size_t code, float volume){
#if 0
    auto& worldpos = pos;
    auto& worldrot = rot;
    
    //create Eigen structures to calculate attenuation
    vraudio::WorldPosition eworldpos(worldpos.x,worldpos.y,worldpos.z);
    vraudio::WorldRotation eroomrot(roomrot.w,roomrot.x,roomrot.y,roomrot.z);
    vraudio::WorldPosition eroompos(roompos.x,roompos.y,roompos.z);
    vraudio::WorldPosition eroomdim(roomDimensions.x,roomDimensions.y,roomDimensions.z);
    auto gain = vraudio::ComputeRoomEffectsGain(eworldpos, eroompos, eroomrot, eroomdim);
            
    // TODO: reuse audio sources across computations
    // get the audio source for the room for this source
    // if one does not exist, create it
    vraudio::ResonanceAudioApi::SourceId src;
    if (!allSources.contains(code)){
        src = audioEngine->CreateSoundObjectSource(vraudio::RenderingMode::kBinauralLowQuality);
        allSources[code] = src;
    }
    else{
        src = allSources[code];
    }
    
    audioEngine->SetInterleavedBuffer(src, data, 1, AudioPlayer::GetBufferSize());   // they copy the contents of temp into their own buffer so giving stack memory is fine here
    audioEngine->SetSourceVolume(src, 1);   // the AudioAsset already applied the volume
    audioEngine->SetSourcePosition(src, worldpos.x, worldpos.y, worldpos.z);
    audioEngine->SetSourceRotation(src, worldrot.x, worldrot.y, worldrot.z, worldrot.w);
    audioEngine->SetSourceRoomEffectsGain(src, gain);
#endif
}


void SimpleAudioSpace::RoomData::Simulate(PlanarSampleBufferInlineView& buffer, PlanarSampleBufferInlineView& scratchBuffer){
#if 0
    auto nchannels = AudioPlayer::GetNChannels();
    
    // convert to an array of pointers for Resonance
    stackarray(allchannelptrs, float*, nchannels);
    for(uint8_t i = 0; i < nchannels; i++){
        allchannelptrs[i] = buffer[i].data();
    }
    
    audioEngine->FillPlanarOutputBuffer(nchannels, buffer.sizeOneChannel(), allchannelptrs);
    AudioGraphComposed::Render(buffer, scratchBuffer, nchannels); // process graph
	
	// destroy sources
	for(const auto& source : allSources){
		audioEngine->DestroySource(source.second);
	}
	allSources.clear();
#endif
}

void RavEngine::SimpleAudioSpace::RoomData::RenderAudioSource(PlanarSampleBufferInlineView& buffer, PlanarSampleBufferInlineView& scratchBuffer, PlanarSampleBufferInlineView monoSourceData, const vector3& sourcePos, entity_t owningEntity, const vector3& listenerPos, const quaternion& listenerRotation)
{
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

    IPLBinauralEffectParams params{};
    params.direction = IPLVector3{ 1.0f, 1.0f, 1.0f }; // TODO: direction from listener to source
    params.hrtf = GetApp()->GetAudioPlayer()->GetSteamAudioHRTF();
    params.interpolation = IPL_HRTFINTERPOLATION_NEAREST;
    params.spatialBlend = 1.0f; 
    params.peakDelays = nullptr;

    iplBinauralEffectApply(binauralEffect, &params, &inBuffer, &outputBuffer);

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
