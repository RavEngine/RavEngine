#include "AudioEngine.hpp"
#include "Entity.hpp"
#include "AudioSource.hpp"

using namespace RavEngine;
using namespace std;

void AudioEngine::SetListenerTransform(const vector3 &worldpos, const quaternion &wr){
	audioEngine->SetHeadPosition(worldpos.x, worldpos.y, worldpos.z);
	audioEngine->SetHeadRotation(wr.x, wr.y, wr.z, wr.w);
}

void AudioEngine::Tick(float fpsScale){
	
}

void AudioEngine::Spawn(Ref<Entity> e){
	if (e->HasComponentOfType<AudioSourceComponent>()){
		auto components = e->GetAllComponentsOfTypeFastPath<AudioSourceComponent>();
		for(auto a : components){
			auto source = static_pointer_cast<AudioSourceComponent>(a);
			source->resonance_handle = audioEngine->CreateSoundObjectSource(vraudio::RenderingMode::kBinauralHighQuality);
		}
			
	}
}

void AudioEngine::Destroy(Ref<Entity> e){
	if (e->HasComponentOfType<AudioSourceComponent>()){
		auto components = e->GetAllComponentsOfTypeFastPath<AudioSourceComponent>();
		for(auto a : components){
			auto source = static_pointer_cast<AudioSourceComponent>(a);
			audioEngine->DestroySource(source->resonance_handle);
			source->resonance_handle = vraudio::ResonanceAudioApi::kInvalidSourceId;
		}
	}
}
