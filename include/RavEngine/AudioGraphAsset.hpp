#pragma once
#include <LabSound/LabSound.h>
#include "AudioTypes.hpp"

namespace RavEngine{

class AudioGraphAsset{
    
    lab::OfflineContext audioContext;
    
public:
    AudioGraphAsset(const lab::AudioStreamConfig& config);
    
    /**
     Render the graph given input samples
     @param inout input samples
     @param scratch buffer
     @param nchannels the number of channels in the buffers
     @post inout will be swapped with scratchBuffer. This is why they are passed by reference.
     */
    void Render(InterleavedSampleBuffer& inout, InterleavedSampleBuffer& scratchBuffer, uint8_t nchannels);
    
    decltype(audioContext.context)& GetContext(){
        return audioContext.context;
    }
};

}
