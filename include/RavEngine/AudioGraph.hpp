#pragma once
#include <LabSound/LabSound.h>
#include "AudioTypes.hpp"

namespace RavEngine{

class AudioGraph{
    
    lab::OfflineContext audioContext;
    
public:
    AudioGraph(const lab::AudioStreamConfig& config);
    
    void Render(const InterleavedSampleBuffer& out);
    
    decltype(audioContext.context)& GetContext(){
        return audioContext.context;
    }
};

}
