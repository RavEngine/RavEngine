#include "AudioGraph.hpp"

using namespace RavEngine;

AudioGraph::AudioGraph(const lab::AudioStreamConfig& config) : audioContext(lab::MakeOfflineAudioContext(config)){
    
}

void AudioGraph::Render(const InterleavedSampleBuffer& out){
    //TODO: set output destination buffer
    
    audioContext.process(out.size());
}
