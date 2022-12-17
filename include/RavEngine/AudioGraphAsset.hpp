#pragma once
#include <LabSound/LabSound.h>
#include "AudioTypes.hpp"

namespace RavEngine{

/**
* Represents an audio effect graph processor. For a list of 
* nodes, see https://developer.mozilla.org/en-US/docs/Web/API/Web_Audio_API
*/
class AudioGraphAsset{
    
    lab::OfflineContext audioContext;
    std::shared_ptr<lab::AudioBus> inputBus, outputBus;
    uint8_t nchannels = 0;
    
public:
    /**
    * Create an AudioGraphAsset.
    * @param nchannels the number of channels. It is dependent on where you want to use this Asset. 
    * If you are using it on point sources, then nchannels should be 1, because pre-spatialized audio is mono. If you are using it after all spatialization is complete, like on the AudioListener, or you are using it on an AmbientAudioSource in which case no processing occurred, then nchannels should be set to the number of output channels for your application. If you are using this asset for something else, then set nchannels accordingly. 
    */
    AudioGraphAsset(uint8_t nchannels);
    
    /**
     Render the graph given input samples
     @param inout input samples
     @param scratch buffer
     @param nchannels the number of channels in the buffers
     @post inout will be swapped with scratchBuffer. This is why they are passed by reference.
     */
    void Render(PlanarSampleBufferInlineView& inout, PlanarSampleBufferInlineView& scratchBuffer, uint8_t nchannels);
    
    decltype(audioContext.context)& GetContext(){
        return audioContext.context;
    }

    auto GetInputNode() const {
        return inputBus;
    }

    auto GetOutputNode() const{
        return outputBus;
    }

    /**
    * Connect two audio nodes within this asset
    * @param source the node providing data
    * @param destination the node to provide data to
    * @note this argument order is reversed comapred to WebAudio
    */
    auto Connect(std::shared_ptr<lab::AudioNode> source, std::shared_ptr<lab::AudioNode> dest);
};

}
