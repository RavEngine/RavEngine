// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/ChannelSplitterNode.h"
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/extended/Registry.h"

#include "internal/Assertions.h"

namespace lab
{

ChannelSplitterNode::ChannelSplitterNode(AudioContext& ac, int numberOfOutputs_)
    : AudioNode(ac)
{
    addInput(std::unique_ptr<AudioNodeInput>(new AudioNodeInput(this)));
    addOutputs(numberOfOutputs_);
    initialize();
}

void ChannelSplitterNode::addOutputs(int numberOfOutputs_)
{
    // Create a fixed number of outputs (able to handle the maximum number of channels fed to an input).
    for (int i = 0; i < numberOfOutputs_; ++i)
    {
        addOutput(std::unique_ptr<AudioNodeOutput>(new AudioNodeOutput(this, 1)));
    }
}

void ChannelSplitterNode::process(ContextRenderLock & r, int bufferSize)
{
    AudioBus * source = input(0)->bus(r);
    ASSERT(source);
    ASSERT_UNUSED(bufferSize, bufferSize == source->length());

    size_t numberOfSourceChannels = source->numberOfChannels();

    for (int i = 0; i < numberOfOutputs(); ++i)
    {
        AudioBus * destination = output(i)->bus(r);
        ASSERT(destination);

        if (i < numberOfSourceChannels)
        {
            // Split the channel out if it exists in the source.
            // It would be nice to avoid the copy and simply pass along pointers, but this becomes extremely difficult with fanout and fanin.
            destination->channel(0)->copyFrom(source->channel(i));
        }
        else if (output(i)->renderingFanOutCount() > 0)
        {
            // Only bother zeroing out the destination if it's connected to anything
            destination->zero();
        }
    }
}

void ChannelSplitterNode::reset(ContextRenderLock &)
{
}

}  // namespace lab
