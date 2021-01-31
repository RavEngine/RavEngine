#include "AudioSource.hpp"
#include "App.hpp"
#include "Debug.hpp"
#include <libnyquist/Decoders.h>
#include <SDL.h>
#include <filesystem>

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
		//TODO: update the sample rate
	}
	
	lengthSeconds = data.lengthSeconds;
	frameSize = data.frameSize;
	numsamples = data.samples.size();
	
	audiodata = new float[data.samples.size()];
	std::memcpy((void*)audiodata, &data.samples[0], data.samples.size() * sizeof(data.samples[0]));
}

AudioAsset::~AudioAsset(){
	delete audiodata;
	audiodata = nullptr;
}
