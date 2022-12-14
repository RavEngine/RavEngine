#pragma once
#include <LabSound/LabSound.h>
#include "AudioTypes.hpp"

namespace RavEngine{

class AudioGraphAsset{
    
    lab::OfflineContext audioContext;
    std::shared_ptr<lab::AudioBus> inputBus, outputBus;
    uint8_t nchannels = 0;
    
public:
    AudioGraphAsset(const lab::AudioStreamConfig& config, uint8_t nchannels);
    
    /**
     Render the graph given input samples
     @param inout input samples
     @param scratch buffer
     @param nchannels the number of channels in the buffers
     @post inout will be swapped with scratchBuffer. This is why they are passed by reference.
     */
    void Render(InterleavedSampleBufferView& inout, InterleavedSampleBufferView& scratchBuffer, uint8_t nchannels);
    
    decltype(audioContext.context)& GetContext(){
        return audioContext.context;
    }
};

}
