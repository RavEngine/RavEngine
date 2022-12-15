#include "AudioGraphAsset.hpp"

using namespace RavEngine;

AudioGraphAsset::AudioGraphAsset(const lab::AudioStreamConfig& config, uint8_t nchannels) :
    audioContext(lab::MakeOfflineAudioContext(config)),
    outputBus(std::make_shared<lab::AudioBus>(nchannels,0,false)),  // we will give it the input data and size when it's time to render
    inputBus(std::make_shared<lab::AudioBus>(nchannels,0, false)),
    nchannels(nchannels)
{
}

void AudioGraphAsset::Render(PlanarSampleBufferInlineView& inout, PlanarSampleBufferInlineView& scratchBuffer, uint8_t nchannels){
    assert(this->nchannels == nchannels);
    
    for (decltype(nchannels) i = 0; i < nchannels; i++){
        inputBus->setChannelMemory(i, inout[i].data(), inout.sizeOneChannel());
        outputBus->setChannelMemory(i, scratchBuffer[i].data(), scratchBuffer.sizeOneChannel());
    }
    
    audioContext.process(inout.sizeOneChannel());
    
    //inout will now have the results of processing
    std::swap(inout, scratchBuffer);
    
}


void AudioGraphComposed::renderImpl(PlanarSampleBufferInlineView inputSamples, PlanarSampleBufferInlineView intermediateBuffer, uint8_t nchannels){
    if (effectGraph){
        effectGraph->Render(inputSamples, intermediateBuffer, nchannels);
    }
}
