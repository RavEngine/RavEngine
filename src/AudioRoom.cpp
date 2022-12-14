#include "AudioRoom.hpp"
#include "Entity.hpp"
#include "AudioSource.hpp"
#include "DataStructures.hpp"
#include "Transform.hpp"
#include "AudioPlayer.hpp"

#include "mathtypes.hpp"
#include <common/room_effects_utils.h>

using namespace RavEngine;
using namespace std;

void AudioRoom::RoomData::SetListenerTransform(const vector3 &worldpos, const quaternion &wr){
	audioEngine->SetHeadPosition(worldpos.x, worldpos.y, worldpos.z);
	audioEngine->SetHeadRotation(wr.x, wr.y, wr.z, wr.w);
}

void AudioRoom::RoomData::AddEmitter(const float* data, const vector3 &pos, const quaternion &rot, const vector3 &roompos, const quaternion &roomrot, size_t code, float volume){
    
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
    
    audioEngine->SetInterleavedBuffer(src, data, 1, NFRAMES);   // they copy the contents of temp into their own buffer so giving stack memory is fine here
    audioEngine->SetSourceVolume(src, 1);   // the AudioAsset already applied the volume
    audioEngine->SetSourcePosition(src, worldpos.x, worldpos.y, worldpos.z);
    audioEngine->SetSourceRotation(src, worldrot.x, worldrot.y, worldrot.z, worldrot.w);
    audioEngine->SetSourceRoomEffectsGain(src, gain);
}

void AudioRoom::RoomData::AddEmitter(AudioPlayerData::Player* source, const vector3& pos, const quaternion& rot, const vector3& roompos, const quaternion& roomrot, size_t nbytes, InterleavedSampleBufferView& effectScratchBuffer){
	if (source->isPlaying){
		
		//get appropriate area in source's buffer if it is playing
        const auto stackarr_size = nbytes/sizeof(float)/AudioPlayer::GetNChannels();
		stackarray(temp, float, stackarr_size);
        InterleavedSampleBufferView view(temp, nbytes/2/sizeof(InterleavedSampleBufferView::value_type));
		source->GetSampleRegionAndAdvance(view, effectScratchBuffer);
		
        AddEmitter(temp, pos, rot, roompos, roomrot, std::hash<decltype(source)>()(source), source->volume);
	}
}

void AudioRoom::RoomData::Simulate(InterleavedSampleBufferView buffer){
    auto nchannels = AudioPlayer::GetNChannels();
	audioEngine->FillInterleavedOutputBuffer(nchannels, buffer.size()/nchannels, buffer.data());
    AudioGraphComposed::Render(buffer,nchannels); // process graph
	
	// destroy sources
	for(const auto& source : allSources){
		audioEngine->DestroySource(source.second);
	}
	allSources.clear();
}

//void RavEngine::AudioRoom::DebugDraw(RavEngine::DebugDrawer& dbg, const RavEngine::Transform& tr) const
//{
//	dbg.DrawRectangularPrism(tr.CalculateWorldMatrix(), debug_color, data->roomDimensions);
//}
