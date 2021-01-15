#include "AudioSyncSystem.hpp"
#include "AudioEngine.hpp"

using namespace RavEngine;

System::list_type AudioSyncSystem::queries;

void AudioSyncSystem::Tick(float fpsScale, Ref<Entity> e){
	//get the App's audio engine
	//call setposition and pass the audio source component
}
