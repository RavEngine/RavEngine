#include "AudioSource.hpp"
#include "App.hpp"
#include "Debug.hpp"
#include <libnyquist/Decoders.h>
#include <libnyquist/Encoders.h>
#include <SDL.h>
#include <filesystem>
#include <r8bbase.h>
#include <CDSPResampler.h>

using namespace RavEngine;
using namespace std;

AudioAsset::AudioAsset(const std::string& name, decltype(nchannels) desired_channels){
	//expand audio into buffer
	string path = StrFormat("/sounds/{}", name);
	auto datavec = App::Resources->FileContentsAt<std::vector<uint8_t>>(path.c_str());
	
	const int desiredSampleRate = 44100;
	
	nqr::NyquistIO loader;
	auto file_ext = filesystem::path(path).extension().string().substr(1);
	nqr::AudioData data;
	loader.Load(&data, file_ext, datavec);
	
	if (data.sampleRate != desiredSampleRate){
		//assume that in release the user intends this behavior, so do not send the warning.
#ifdef _DEBUG
		Debug::Warning("Sample rate mismatch in {} - requested {} hz but got {} hz. Beginning resampling - to reduce load times, supply audio in the correct sample rate.", path, desiredSampleRate, data.sampleRate);
#endif

		// resample to the correct rate
		decltype(data.samples) finalbuffer(desiredSampleRate * data.lengthSeconds * data.channelCount);
		decltype(data.samples) oneChannel(data.sampleRate * data.lengthSeconds);
		decltype(data.samples) outChannel(desiredSampleRate * data.lengthSeconds);
		for (int i = 0; i < data.channelCount; i++) {
			// extract the channel
			for (int j = 0; j < oneChannel.size(); j++) {
				oneChannel[j] = data.samples[j * data.channelCount + i];
			}

			r8b::CDSPResampler resampler(data.sampleRate, desiredSampleRate, oneChannel.size());
			resampler.oneshot(oneChannel.data(), oneChannel.size(), outChannel.data(), outChannel.size());

			// merge into final buffer
			for (int j = 0; j < outChannel.size(); j++) {
				finalbuffer[j * data.channelCount + i] = outChannel[j];
			}
		}

		//update properties
		data.sampleRate = desiredSampleRate;
		data.samples = finalbuffer;
	}

	// fix n channels
	if (data.channelCount != desired_channels) {
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
			Debug::Fatal("Unable to convert input audio with {} channels to desired {} channels",data.channelCount,desired_channels);
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
