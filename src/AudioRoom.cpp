#include "AudioRoom.hpp"
#include "Entity.hpp"
#include "AudioSource.hpp"
#include "DataStructures.hpp"
#include "Transform.hpp"

#include "mathtypes.hpp"
#include <common/room_effects_utils.h>

using namespace RavEngine;
using namespace std;

void AudioRoom::RoomData::SetListenerTransform(const vector3 &worldpos, const quaternion &wr){
	audioEngine->SetHeadPosition(worldpos.x, worldpos.y, worldpos.z);
	audioEngine->SetHeadRotation(wr.x, wr.y, wr.z, wr.w);
}

void AudioRoom::RoomData::AddEmitter(AudioPlayerData::Player* source, const vector3& pos, const quaternion& rot, const vector3& roompos, const quaternion& roomrot, size_t nbytes){
	if (source->isPlaying){
		auto& worldpos = pos;
		auto& worldrot = rot;
		
		//get appropriate area in source's buffer if it is playing
        const auto stackarr_size = nbytes/sizeof(float)/2;
		stackarray(temp, float, stackarr_size);
		source->GetSampleRegionAndAdvance(temp, nbytes/2);
		
		//create Eigen structures to calculate attenuation
		vraudio::WorldPosition eworldpos(worldpos.x,worldpos.y,worldpos.z);
		vraudio::WorldRotation eroomrot(roomrot.w,roomrot.x,roomrot.y,roomrot.z);
		vraudio::WorldPosition eroompos(roompos.x,roompos.y,roompos.z);
		vraudio::WorldPosition eroomdim(roomDimensions.x,roomDimensions.y,roomDimensions.z);
		auto gain = vraudio::ComputeRoomEffectsGain(eworldpos, eroompos, eroomrot, eroomdim);
		
		//mix results
		for(int i = 0; i < stackarr_size; i++){
			temp[i] *= gain;		//apply attenuation to source
		}
		
		// TODO: reuse audio sources across computations
		// get the audio source for the room for this source
		// if one does not exist, create it
		auto code = std::hash<decltype(source)>()(source);
		vraudio::ResonanceAudioApi::SourceId src;
		if (!allSources.contains(code)){
			src = audioEngine->CreateSoundObjectSource(vraudio::RenderingMode::kBinauralLowQuality);
			allSources[code] = src;
		}
		else{
			src = allSources[code];
		}
		
		audioEngine->SetInterleavedBuffer(src, temp, 1, NFRAMES);
		audioEngine->SetSourceVolume(src, source->volume);
		audioEngine->SetSourcePosition(src, worldpos.x, worldpos.y, worldpos.z);
		audioEngine->SetSourceRotation(src, worldrot.x, worldrot.y, worldrot.z, worldrot.w);
	}
}

void AudioRoom::RoomData::Simulate(float *ptr, size_t nbytes){
	audioEngine->FillInterleavedOutputBuffer(2, NFRAMES, ptr);
	
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
