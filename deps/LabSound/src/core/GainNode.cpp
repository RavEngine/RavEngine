// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/GainNode.h"
#include "LabSound/core/AudioArray.h"
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioNodeOutput.h"

#include "LabSound/extended/AudioContextLock.h"
#include "LabSound/extended/Registry.h"

#include "internal/Assertions.h"

namespace lab
{

GainNode::GainNode(AudioContext& ac)
    : AudioNode(ac)
    , m_lastGain(1.f)
    , m_sampleAccurateGainValues(AudioNode::ProcessingSizeInFrames)  // FIXME: can probably share temp buffer in context
{
    addInput(std::unique_ptr<AudioNodeInput>(new AudioNodeInput(this)));
    addOutput(std::unique_ptr<AudioNodeOutput>(new AudioNodeOutput(this, 1)));

    m_gain = std::make_shared<AudioParam>("gain", "GAIN", 1.0, 0.0, 10000.0);
    m_params.push_back(m_gain);

    initialize();
}

GainNode::~GainNode()
{
    uninitialize();
}

void GainNode::process(ContextRenderLock &r, int bufferSize)
{
    // FIXME: for some cases there is a nice optimization to avoid processing here, and let the gain change
    // happen in the summing junction input of the AudioNode we're connected to.
    // Then we can avoid all of the following:

    AudioBus * outputBus = output(0)->bus(r);
    ASSERT(outputBus);


    if (!isInitialized() || !input(0)->isConnected())
    {
        outputBus->zero();
        return;
    }


    AudioBus* inputBus = input(0)->bus(r);
    const int inputBusChannelCount = inputBus->numberOfChannels();
    if (!inputBusChannelCount)
    {
        outputBus->zero();
        return;
    }

    int outputBusChannelCount = outputBus->numberOfChannels();
    if (inputBusChannelCount != outputBusChannelCount)
    {
        output(0)->setNumberOfChannels(r, inputBusChannelCount);
        outputBusChannelCount = inputBusChannelCount;
        outputBus = output(0)->bus(r);
    }

    if (gain()->hasSampleAccurateValues())
    {
        // Apply sample-accurate gain scaling for precise envelopes, grain windows, etc.
        ASSERT(bufferSize <= m_sampleAccurateGainValues.size());
        if (bufferSize <= m_sampleAccurateGainValues.size())
        {
            float* gainValues_base = m_sampleAccurateGainValues.data();
            float* gainValues = gainValues_base + _scheduler._renderOffset;
            gain()->calculateSampleAccurateValues(r, gainValues, _scheduler._renderLength);
            if (_scheduler._renderOffset > 0)
                memset(gainValues_base, 0, sizeof(float) * _scheduler._renderOffset);
            int bzero_start = _scheduler._renderOffset + _scheduler._renderLength;
            if (bzero_start < bufferSize)
                memset(gainValues_base + bzero_start, 0, sizeof(float) * bufferSize - bzero_start);
            outputBus->copyWithSampleAccurateGainValuesFrom(*inputBus, m_sampleAccurateGainValues.data(), bufferSize);
        }
    }
    else
    {
        // Apply the gain with de-zippering into the output bus.
        outputBus->copyWithGainFrom(*inputBus, &m_lastGain, gain()->value());
    }

    outputBus->clearSilentFlag();
}

void GainNode::reset(ContextRenderLock & r)
{
    // Snap directly to desired gain.
    m_lastGain = gain()->value();
}


}  // namespace lab
