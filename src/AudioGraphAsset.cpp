#include "AudioGraphAsset.hpp"
#include "AudioPlayer.hpp"

using namespace RavEngine;

AudioGraphAsset::AudioGraphAsset(uint8_t nchannels) :
    audioContext(lab::MakeOfflineAudioContext(lab::AudioStreamConfig{
            .desired_channels = nchannels,
            .desired_samplerate = static_cast<float>(AudioPlayer::GetSamplesPerSec()),
        })),
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

auto RavEngine::AudioGraphAsset::Connect(std::shared_ptr<lab::AudioNode> source, std::shared_ptr<lab::AudioNode> dest)
{
    audioContext.context->connect(dest, source);
}



void AudioGraphComposed::renderImpl(PlanarSampleBufferInlineView inputSamples, PlanarSampleBufferInlineView intermediateBuffer, uint8_t nchannels){
    if (effectGraph){
        effectGraph->Render(inputSamples, intermediateBuffer, nchannels);
    }
}
