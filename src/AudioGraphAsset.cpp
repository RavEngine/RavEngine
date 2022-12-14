#include "AudioGraphAsset.hpp"

using namespace RavEngine;

AudioGraphAsset::AudioGraphAsset(const lab::AudioStreamConfig& config, uint8_t nchannels) :
    audioContext(lab::MakeOfflineAudioContext(config)),
    outputBus(std::make_shared<lab::AudioBus>(nchannels,0,false)),  // we will give it the input data and size when it's time to render
    inputBus(std::make_shared<lab::AudioBus>(nchannels,0, false)),
    nchannels(nchannels)
{
}

void AudioGraphAsset::Render(InterleavedSampleBufferView& inout, InterleavedSampleBufferView& scratchBuffer, uint8_t nchannels){
    assert(this->nchannels == nchannels);
    
    //TODO: setchannelmemory for input and output
    
    audioContext.process(inout.size());
    
    //inout will now have the results of processing
    std::swap(inout, scratchBuffer);
    
}


void AudioGraphComposed::renderImpl(InterleavedSampleBufferView inputSamples, InterleavedSampleBufferView intermediateBuffer, uint8_t nchannels){
    if (effectGraph){
        effectGraph->Render(inputSamples, intermediateBuffer, nchannels);
    }
}
