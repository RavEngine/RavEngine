#include "AudioRoom.hpp"
#include "Entity.hpp"
#include "AudioSource.hpp"
#include "DataStructures.hpp"
#include <common/room_effects_utils.h>

using namespace RavEngine;
using namespace std;

void AudioRoom::SetListenerTransform(const vector3 &worldpos, const quaternion &wr){
	audioEngine->SetHeadPosition(worldpos.x, worldpos.y, worldpos.z);
	audioEngine->SetHeadRotation(wr.x, wr.y, wr.z, wr.w);
}

void AudioRoom::Simulate(float *ptr, size_t nbytes, const ComponentStore<SpinLock>::entry_type &sources){

	for(const auto& s : sources){
		Ref<AudioSourceComponent> source = static_pointer_cast<AudioSourceComponent>(s);
		Ref<Entity> owner = source->getOwner().lock();
		if (owner && owner->IsInWorld() && source->IsPlaying()){
			auto worldpos = owner->transform()->GetWorldPosition();
			auto worldrot = owner->transform()->GetWorldRotation();
			
			SimulateSingle(ptr, nbytes, source.get(), worldpos, worldrot);
		}
	}
}

void AudioRoom::SimulateSingle(float *ptr, size_t nbytes, AudioPlayerData* source, const vector3 &worldpos, const quaternion &worldrot){
	auto owner = getOwner().lock();
	if (owner){
		stackarray(temp, float, nbytes/sizeof(float)/2);
		stackarray(outtemp, float, nbytes/sizeof(float));
		
		//get appropriate area in source's buffer if it is playing
		source->GetSampleRegionAndAdvance(temp, nbytes/2);
		
		audioEngine->SetInterleavedBuffer(src, temp, 1, NFRAMES);
		audioEngine->SetSourceVolume(src, source->GetVolume());
		audioEngine->SetSourcePosition(src, worldpos.x, worldpos.y, worldpos.z);
		audioEngine->SetSourceRotation(src, worldrot.x, worldrot.y, worldrot.z, worldrot.w);
		
		audioEngine->FillInterleavedOutputBuffer(2, NFRAMES, outtemp);
		
		auto roompos = owner->transform()->GetWorldPosition();
		auto roomrot = owner->transform()->GetWorldRotation();
		
		//create Eigen structures to calculate attenuation
		vraudio::WorldPosition eworldpos(worldpos.x,worldpos.y,worldpos.z);
		vraudio::WorldRotation eroomrot(roomrot.w,roomrot.x,roomrot.y,roomrot.z);
		vraudio::WorldPosition eroompos(roompos.x,roompos.y,roompos.z);
		vraudio::WorldPosition eroomdim(roomDimensions.x,roomDimensions.y,roomDimensions.z);
		auto gain = vraudio::ComputeRoomEffectsGain(eworldpos, eroompos, eroomrot, eroomdim);
		
		//mix results
		for(int i = 0; i < nbytes/sizeof(float); i++){
			ptr[i] += outtemp[i] * gain;		//apply attenuation
		}
	}
}
