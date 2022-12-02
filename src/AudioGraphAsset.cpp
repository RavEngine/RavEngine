#include "AudioGraphAsset.hpp"

using namespace RavEngine;

AudioGraphAsset::AudioGraphAsset(const lab::AudioStreamConfig& config) : audioContext(lab::MakeOfflineAudioContext(config)){
    
}

void AudioGraphAsset::Render(InterleavedSampleBuffer& inout, InterleavedSampleBuffer& scratchBuffer){
    //TODO: set output destination buffer as scratchBuffer
    //TODO: set input buffer as inout
    
    audioContext.process(inout.size());
    
    //inout will now have the results of processing
    std::swap(inout, scratchBuffer);
    
}


void AudioGraphComposed::renderImpl(InterleavedSampleBuffer inputSamples, InterleavedSampleBuffer intermediateBuffer){
    if (effectGraph){
        effectGraph->Render(inputSamples, intermediateBuffer);
    }
}
