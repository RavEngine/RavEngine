#include "AudioSyncSystem.hpp"
#include "AudioEngine.hpp"

using namespace RavEngine;
using namespace std;

System::list_type AudioSyncSystem::queries;

void AudioSyncSystem::Tick(float fpsScale, Ref<Entity> e){
	auto sources = e->GetAllComponentsOfType<AudioSourceComponent>();
	//get the App's audio engine
	
	for(const auto a : sources){
		auto source = static_pointer_cast<AudioSourceComponent>(a);
		auto pos = e->transform()->GetWorldPosition();
		auto rot = e->transform()->GetWorldRotation();
		
		//call setposition / setrotation and pass the audio source component to the AudioEngine
		engptr.audioEngine->SetSourcePosition(source->resonance_handle, pos.x, pos.y, pos.z);
		engptr.audioEngine->SetSourceRotation(source->resonance_handle, rot.x, rot.y, rot.z, rot.w);
		engptr.audioEngine->SetSourceVolume(source->resonance_handle, source->volume);
	}
}
