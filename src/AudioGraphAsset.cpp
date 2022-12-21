#include "AudioGraphAsset.hpp"
#include "AudioPlayer.hpp"

using namespace RavEngine;

AudioGraphAsset::AudioGraphAsset(uint8_t nchannels) :
    audioContext(lab::MakeOfflineAudioContext(lab::AudioStreamConfig{
            .desired_channels = nchannels,
            .desired_samplerate = static_cast<float>(AudioPlayer::GetSamplesPerSec()),
        })),
    outputBus(std::make_shared<decltype(outputBus)::element_type>(nchannels,0,false)),  // we will give it the input data and size when it's time to render
    inputBus(std::make_shared<decltype(inputBus)::element_type>(nchannels,0, false)),
    inputNode(std::make_shared<decltype(inputNode)::element_type>(*audioContext.context.get())),
    outputNode(std::make_shared<decltype(outputNode)::element_type>(*audioContext.context.get(),nchannels)),
    nchannels(nchannels)
{
    lab::ContextRenderLock r(audioContext.context.get(), "Setup bus");
    inputNode->setBus(r,inputBus);
    audioContext.context->addAutomaticPullNode(outputNode);
    outputNode->setBus(outputBus);
}

void AudioGraphAsset::Render(PlanarSampleBufferInlineView& inout, PlanarSampleBufferInlineView& scratchBuffer, uint8_t nchannels){
    assert(this->nchannels == nchannels);
    
    for (decltype(nchannels) i = 0; i < nchannels; i++){
        inputBus->setChannelMemory(i, inout[i].data(), inout.sizeOneChannel());
        outputBus->setChannelMemory(i, scratchBuffer[i].data(), scratchBuffer.sizeOneChannel());
    }
    inputNode->schedule(0);
    
    audioContext.process(inout.sizeOneChannel());
    
    //inout will now have the results of processing
    std::swap(inout, scratchBuffer);    
}

void RavEngine::AudioGraphAsset::Connect(std::shared_ptr<lab::AudioNode> source, std::shared_ptr<lab::AudioNode> dest)
{
    audioContext.context->connect(dest, source);
}



void AudioGraphComposed::renderImpl(PlanarSampleBufferInlineView inputSamples, PlanarSampleBufferInlineView intermediateBuffer, uint8_t nchannels){
    if (effectGraph){
        effectGraph->Render(inputSamples, intermediateBuffer, nchannels);
    }
}

AudioGraphAsset::LiveCaptureNode::LiveCaptureNode(lab::AudioContext& r, int channelcount) : AudioNode(r) {
    m_channelCount = channelcount;
    m_channelCountMode = lab::ChannelCountMode::Explicit;
    m_channelInterpretation = lab::ChannelInterpretation::Discrete;
    addInput(std::make_unique<lab::AudioNodeInput>(this));
    addOutput(std::make_unique<lab::AudioNodeOutput>(this,1));  //TODO: can we prevent people from connecting this as an output?
    initialize();
}

void AudioGraphAsset::LiveCaptureNode::process(lab::ContextRenderLock& r, int bufferSize) {
    // adapted from lab::RecorderNode::process

    lab::AudioBus* inputBus = input(0)->bus(r);

    bool has_input = inputBus != nullptr && input(0)->isConnected() && inputBus->numberOfChannels() > 0;
    if ((!isInitialized() || !has_input) && outputBus)
    {
        outputBus->zero();
    }

    if (!has_input)
    {
        // nothing to record.
        if (outputBus)
            outputBus->zero();
        return;
    }

    const int inputBusNumChannels = inputBus->numberOfChannels();
    assert(inputBusNumChannels == m_channelCount);  // can only capture if the config matches

    // this puts the data back in our memory rather than sitting in labsound's memory
    if (outputBus) {
        for (int c = 0; c < inputBusNumChannels; c++) {
            const auto maxsamples = outputBus->length() - renderedSoFar;
#pragma omp simd
            for (int i = 0; i < std::min<int>(bufferSize,maxsamples); i++) {
                auto sampleIdx = renderedSoFar + i;
                auto sample = inputBus->channel(c)->data()[i];
                outputBus->channel(c)->mutableData()[sampleIdx] = sample;
            }
        }
        renderedSoFar += bufferSize;
        if (renderedSoFar >= outputBus->length()) {
            renderedSoFar = 0;
        }
    }
}