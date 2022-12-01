#pragma once
#include <LabSound/LabSound.h>
#include "AudioTypes.hpp"

namespace RavEngine{

class AudioGraph{
    
    lab::OfflineContext audioContext;
    
public:
    AudioGraph(const lab::AudioStreamConfig& config);
    
    /**
     Render the graph given input samples
     @param inout input samples
     @param scratch buffer
     @post inout will be swapped with scratchBuffer. This is why they are passed by reference.
     */
    void Render(InterleavedSampleBuffer& inout, InterleavedSampleBuffer& scratchBuffer);
    
    decltype(audioContext.context)& GetContext(){
        return audioContext.context;
    }
};

}
