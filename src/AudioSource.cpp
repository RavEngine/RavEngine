#include "AudioSource.hpp"
#include "App.hpp"
#include "Debug.hpp"
#include <libnyquist/Decoders.h>

using namespace RavEngine;
using namespace std;

AudioAsset::AudioAsset(const std::string& name){
	//expand audio into buffer
	string path = fmt::format("/sounds/{}", name);
	vector<uint8_t> datavec;
	App::Resources->FileContentsAt(path.c_str(), datavec);
	
	const int desiredSampleRate = 44100;
	
	nqr::NyquistIO loader;
	auto file_ext = filesystem::path(path).extension().string().substr(1);
	nqr::AudioData data;
	loader.Load(&data, file_ext, datavec);
	if (data.channelCount != 1){
		Debug::Fatal("Only mono is supported, got {} channels", data.channelCount);
	}
	
	if (data.sampleRate != desiredSampleRate){
		Debug::Warning("Sample rate mismatch in {} - requested {} hz but got {} hz", path, desiredSampleRate, data.sampleRate);
	}
	
	lengthSeconds = data.lengthSeconds;
	frameSize = data.frameSize;
	
	audiodata[0] = new float[data.samples.size()];
	std::memcpy((void*)audiodata[0], &data.samples[0], data.samples.size() * sizeof(data.samples[0]));
}

AudioAsset::~AudioAsset(){
	delete audiodata[0];
}


void AudioSourceComponent::Tick(float scale){
	//convert to a time delta
	if (isPlaying){
		double timeDelta = 1.0/App::evalNormal * scale * playbackSpeed;
		
		//advance the playhead
		playhead_pos += timeDelta;
		
		//if looping, wrap around the playhead, otherwise clamp to end
		if (loops){
			playhead_pos = playhead_pos - asset->getLength();
			//TODO: will also need to swap pointer to the version with overlapping end->begin to achieve gapless loop
		}
		else{
			playhead_pos = asset->getLength();
		}
		
	}
}
