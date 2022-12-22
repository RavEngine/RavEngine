#include "AudioGraphAsset.hpp"
#include "AudioPlayer.hpp"

using namespace RavEngine;

AudioGraphAsset::AudioGraphAsset(uint8_t nchannels) :
    nchannels(nchannels)
{
   
}

void AudioGraphAsset::Render(PlanarSampleBufferInlineView& inout, PlanarSampleBufferInlineView& scratchBuffer, uint8_t nchannels){
    assert(this->nchannels == nchannels);
    
    // iterate the stack
    for (auto& filter : filters) {
        // call filter
        filter->process(inout, scratchBuffer);
        
        //inout will now have the results of processing
        std::swap(inout, scratchBuffer);
    }
}


void AudioGraphComposed::renderImpl(PlanarSampleBufferInlineView& inputSamples, PlanarSampleBufferInlineView& intermediateBuffer, uint8_t nchannels){
    if (effectGraph){
        effectGraph->Render(inputSamples, intermediateBuffer, nchannels);
    }
}