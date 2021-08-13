#include "AudioSource.hpp"
#include "App.hpp"
#include "Debug.hpp"
#include <libnyquist/Decoders.h>
#include <libnyquist/Encoders.h>
#include <SDL.h>
#include <filesystem>

using namespace RavEngine;
using namespace std;

AudioAsset::AudioAsset(const std::string& name, decltype(nchannels) desired_channels){
	//expand audio into buffer
	string path = StrFormat("/sounds/{}", name);
	vector<uint8_t> datavec;
	App::Resources->FileContentsAt(path.c_str(), datavec);
	
	const int desiredSampleRate = 44100;
	
	nqr::NyquistIO loader;
	auto file_ext = filesystem::path(path).extension().string().substr(1);
	nqr::AudioData data;
	loader.Load(&data, file_ext, datavec);
	
	if (data.sampleRate != desiredSampleRate){
		Debug::Warning("Sample rate mismatch in {} - requested {} hz but got {} hz", path, desiredSampleRate, data.sampleRate);
	}

	// fix n channels
	if (nchannels != desired_channels) {
		// mono -> Stereo
		if (desired_channels == 2 && data.channelCount == 1) {
			decltype(data.samples) newSamples(data.samples.size() * 2);
			nqr::MonoToStereo(data.samples.data(),newSamples.data(), data.samples.size());
			data.samples = newSamples;
		}
		else if (desired_channels == 1 && data.channelCount == 2) {
			decltype(data.samples) newSamples(data.samples.size() / 2);
			nqr::StereoToMono(data.samples.data(), newSamples.data(), data.samples.size());
			data.samples = newSamples;
		}
		else {
			Debug::Fatal("Unable to convert input audio with {} channels to desired {} channels",nchannels,desired_channels);
		}

		nchannels = desired_channels;
	}
	else {
		nchannels = data.channelCount;
	}

	
	lengthSeconds = data.lengthSeconds;
	frameSize = data.frameSize;
	numsamples = data.samples.size();
	
	audiodata = new float[data.samples.size()];
	std::memcpy((void*)audiodata, &data.samples[0], data.samples.size() * sizeof(data.samples[0]));
}

AudioAsset::~AudioAsset(){
	delete[] audiodata;
	audiodata = nullptr;
}
