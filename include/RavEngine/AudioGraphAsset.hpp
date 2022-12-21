#pragma once
#include <LabSound/LabSound.h>
#include "AudioTypes.hpp"

namespace RavEngine{

/**
* Represents an audio effect graph processor. For a list of 
* nodes, see https://developer.mozilla.org/en-US/docs/Web/API/Web_Audio_API
*/
class AudioGraphAsset{
    class LiveCaptureNode;

    lab::OfflineContext audioContext;
    std::shared_ptr<lab::AudioBus> inputBus, outputBus;
    std::shared_ptr<lab::SampledAudioNode> inputNode;
    std::shared_ptr<LiveCaptureNode> outputNode;
    uint8_t nchannels = 0;
    
public:

    class LiveCaptureNode : public lab::AudioNode {
        virtual double tailTime(lab::ContextRenderLock& r) const final { return 0; }
        virtual double latencyTime(lab::ContextRenderLock& r) const final { return 0; }
        std::shared_ptr<lab::AudioBus> outputBus;
        int renderedSoFar = 0;
    public:
        LiveCaptureNode(lab::AudioContext& r, int channelcount);
        virtual ~LiveCaptureNode() {
            uninitialize();
        }

        constexpr static const char* static_name() { return "RVELiveCaptureNode"; }
        virtual const char* name() const override { return static_name(); }

        void setBus(decltype(outputBus) bus) {
            outputBus = bus;
        }

        // AudioNode
        virtual void process(lab::ContextRenderLock&, int bufferSize) override;
        virtual void reset(lab::ContextRenderLock&) override {
            renderedSoFar = 0;
        } 
    };


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
        return inputNode;
    }

    auto GetOutputNode() const{
        return outputNode;
    }

    /**
    * Connect two audio nodes within this asset
    * @param source the node providing data
    * @param destination the node to provide data to
    * @note this argument order is reversed comapred to WebAudio
    */
    void Connect(std::shared_ptr<lab::AudioNode> source, std::shared_ptr<lab::AudioNode> dest);

    /**
    * Create a node within this context. This node may only be used within this context.
    * @param args parameters for the node
    */
    template<typename T, typename ... A>
    std::shared_ptr<T> CreateNode(A&& ... args) {
        return std::make_shared<T>(*audioContext.context.get(), args...);
    }
};

}
