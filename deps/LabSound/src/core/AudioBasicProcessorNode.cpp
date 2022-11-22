// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/AudioBasicProcessorNode.h"
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/AudioProcessor.h"

#include "internal/Assertions.h"

namespace lab
{

AudioBasicProcessorNode::AudioBasicProcessorNode(AudioContext & ac)
    : AudioNode(ac)
{
    addInput(std::unique_ptr<AudioNodeInput>(new AudioNodeInput(this)));
    addOutput(std::unique_ptr<AudioNodeOutput>(new AudioNodeOutput(this, 1)));

    // The subclass must create m_processor.
}

void AudioBasicProcessorNode::initialize()
{
    if (isInitialized())
        return;

    ASSERT(processor());
    processor()->initialize();

    AudioNode::initialize();
}

void AudioBasicProcessorNode::uninitialize()
{
    if (!isInitialized())
        return;

    ASSERT(processor());
    processor()->uninitialize();

    AudioNode::uninitialize();
}

void AudioBasicProcessorNode::process(ContextRenderLock & r, int bufferSize)
{
    AudioBus * destinationBus = output(0)->bus(r);

    if (!isInitialized() || !processor())
        destinationBus->zero();
    else
    {
        AudioBus * sourceBus = input(0)->bus(r);

        // FIXME: if we take "tail time" into account, then we can avoid calling processor()->process() once the tail dies down.
        if (!input(0)->isConnected())
        {
            sourceBus->zero();
            return;
        }

        int srcChannelCount = sourceBus->numberOfChannels();
        int dstChannelCount = destinationBus->numberOfChannels();
        if (srcChannelCount != dstChannelCount)
        {
            output(0)->setNumberOfChannels(r, srcChannelCount);
            destinationBus = output(0)->bus(r);
        }

        // process entire buffer
        processor()->process(r, sourceBus, destinationBus, bufferSize);
    }
}


void AudioBasicProcessorNode::reset(ContextRenderLock &)
{
    if (processor())
        processor()->reset();
}

int AudioBasicProcessorNode::numberOfChannels()
{
    return output(0)->numberOfChannels();
}

double AudioBasicProcessorNode::tailTime(ContextRenderLock & r) const
{
    return m_processor->tailTime(r);
}

double AudioBasicProcessorNode::latencyTime(ContextRenderLock & r) const
{
    return m_processor->latencyTime(r);
}

AudioProcessor * AudioBasicProcessorNode::processor()
{
    return m_processor.get();
}

AudioProcessor * AudioBasicProcessorNode::processor() const
{
    return m_processor.get();
}

}  // namespace lab
