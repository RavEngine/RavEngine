#pragma once
#include "AudioTypes.hpp"
#include "DataStructures.hpp"
#include "Function.hpp"

namespace RavEngine{

struct AudioFilterLayer {
    virtual void process(const PlanarSampleBufferInlineView, PlanarSampleBufferInlineView) = 0;
};

/**
* A simple gain effect layer to illustrate the API
*/
struct AudioGainFilterLayer : public AudioFilterLayer {
    float gain = 1;
    void process(const PlanarSampleBufferInlineView in, PlanarSampleBufferInlineView out) final {
        for (int c = 0; c < in.GetNChannels(); c++) {
            auto channel = in[c];
#pragma omp simd
            for (int i = 0; i < channel.size(); i++) {
                out[c][i] = channel[i] * gain;
            }
        }
    }
    AudioGainFilterLayer() {}
    AudioGainFilterLayer(float gain) : gain(gain) {}
};

/**
* Represents an audio effect graph processor. For a list of 
* nodes, see https://developer.mozilla.org/en-US/docs/Web/API/Web_Audio_API
*/
class AudioGraphAsset{
   
    uint8_t nchannels = 0;
    
public:

    LinkedList<std::shared_ptr<AudioFilterLayer>> filters;

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
    
};

}
