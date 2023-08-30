#if !RVE_SERVER
#if defined _M_ARM64 && _M_ARM64
#define ARCH_CPU_LITTLE_ENDIAN 1
#endif
#include "AudioSource.hpp"
#include "App.hpp"
#include "Debug.hpp"
#include <libnyquist/Decoders.h>
#include <libnyquist/Encoders.h>
#include <SDL.h>
#include "Filesystem.hpp"
#include <r8bbase.h>
#include <CDSPResampler.h>
#include "VirtualFileSystem.hpp"
#include "AudioPlayer.hpp"

using namespace RavEngine;
using namespace std;

SampledAudioDataProvider::SampledAudioDataProvider(decltype(asset) a, uint8_t nchannels) : asset(a), AudioDataProvider(AudioPlayer::GetBufferCount(), AudioPlayer::GetBufferSize(), nchannels){}

AudioSourceComponent::AudioSourceComponent(Ref<AudioDataProvider> a) : AudioSourceBase(a){}

AmbientAudioSourceComponent::AmbientAudioSourceComponent(Ref<AudioDataProvider> a)  : AudioSourceBase(a) {}

InstantaneousAudioSource::InstantaneousAudioSource(Ref<AudioAsset> a, const vector3& position, float vol) : AudioSourceBase(New<SampledAudioDataProvider>(a)), source_position(position){
    player->SetVolume(vol);
    player->Play();
}

InstantaneousAmbientAudioSource::InstantaneousAmbientAudioSource(Ref<AudioAsset> a, float vol) : AudioSourceBase(New<SampledAudioDataProvider>(a)) {
    player->SetVolume(vol);
    player->Play();
}

AudioAsset::AudioAsset(const std::string& name, decltype(nchannels) desired_channels){
	//expand audio into buffer
	string path = StrFormat("/sounds/{}", name);
	auto datavec = GetApp()->GetResources().FileContentsAt<std::vector<uint8_t>>(path.c_str(),false);    // the extra arg signals not to null terminate the file data
	
	const int desiredSampleRate = AudioPlayer::GetSamplesPerSec();
	
	nqr::NyquistIO loader;
	auto file_ext = Filesystem::Path(path).extension().string().substr(1);
	nqr::AudioData data;
	loader.Load(&data, file_ext, datavec);
	
	if (data.sampleRate != desiredSampleRate){
		//assume that in release the user intends this behavior, so do not send the warning.
#ifndef NDEBUG
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

			r8b::CDSPResampler resampler(data.sampleRate, desiredSampleRate, Debug::AssertSize<int>(oneChannel.size()));
			resampler.oneshot(oneChannel.data(), static_cast<int>(oneChannel.size()), outChannel.data(), static_cast<int>(oneChannel.size()));

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
        // stereo -> mono
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
	
    audiodata = new float[data.samples.size()]{0};
    
    // convert to planar representation
    PlanarSampleBufferInlineView planarRep{const_cast<float*>(audiodata),data.samples.size(),data.samples.size() / nchannels};
	planarRep.ImportInterleavedData(InterleavedSampleBufferView{data.samples.data(),data.samples.size()}, nchannels);
    this->data = planarRep;
}

AudioAsset::~AudioAsset(){
	delete[] audiodata;
	audiodata = nullptr;
}

using namespace RavEngine;

RavEngine::PlanarSampleBufferInlineView AudioRenderBuffer::SingleRenderBuffer::GetDataBufferView() const { 
    return PlanarSampleBufferInlineView{data_impl, AudioPlayer::GetBufferSize(), static_cast<size_t>(AudioPlayer::GetBufferSize() / nchannels)};
}

RavEngine::PlanarSampleBufferInlineView AudioRenderBuffer::SingleRenderBuffer::GetScratchBufferView() const { 
    return PlanarSampleBufferInlineView{scratch_impl, AudioPlayer::GetBufferSize(), static_cast<size_t>(AudioPlayer::GetBufferSize() / nchannels)};
}


void SampledAudioDataProvider::ProvideBufferData(PlanarSampleBufferInlineView& buffer, PlanarSampleBufferInlineView& scratchSpace) {
    const auto nsamples = asset->GetNumSamples();
    const auto nchannels = asset->GetNChanels();
    assert(buffer.GetNChannels() >= nchannels);  // you are trying to do something that doesn't make sense!!
    for(size_t i = 0; i < buffer.sizeOneChannel(); i++){
        //is playhead past end of source?
        if (playhead_pos >= nsamples){
            if (loops){
                playhead_pos = 0;
            }
            else{
#pragma omp simd
                for(uint8_t c = 0; c < nchannels; c++){
                    buffer[c][i] = 0;
                }
                isPlaying = false;
                continue;
            }
        }
#pragma omp simd
        for(uint8_t c = 0; c < nchannels; c++){
            buffer[c][i] = asset->data[c][playhead_pos] * volume;
        }
        playhead_pos++;
    }
    AudioGraphComposed::Render(buffer,scratchSpace, asset->GetNChanels());
}
#endif
