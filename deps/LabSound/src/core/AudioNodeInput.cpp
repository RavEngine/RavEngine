// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/core/AudioNode.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/Mixing.h"

#include "LabSound/extended/AudioContextLock.h"

#include "internal/Assertions.h"

#include <algorithm>
#include <mutex>

using namespace std;

namespace lab
{

AudioNodeInput::AudioNodeInput(AudioNode * node, int processingSizeInFrames)
    : AudioSummingJunction()
    , m_destinationNode(node)
{
    // Set to mono by default.
    m_internalSummingBus = std::unique_ptr<AudioBus>(new AudioBus(Channels::Mono, processingSizeInFrames));
}

AudioNodeInput::~AudioNodeInput()
{
}

void AudioNodeInput::connect(ContextGraphLock & g, std::shared_ptr<AudioNodeInput> junction, std::shared_ptr<AudioNodeOutput> toOutput)
{
    if (!junction || !toOutput || !junction->destinationNode())
        return;

    // return if input is already connected to this output.
    if (junction->isConnected(toOutput)) return;

    toOutput->addInput(g, junction);
    junction->junctionConnectOutput(toOutput);
}

void AudioNodeInput::disconnect(ContextGraphLock & g, std::shared_ptr<AudioNodeInput> junction, std::shared_ptr<AudioNodeOutput> toOutput)
{
    ASSERT(g.context());
    if (!junction || !junction->destinationNode() || !toOutput)
        return;

    if (junction->isConnected(toOutput))
    {
        junction->junctionDisconnectOutput(toOutput);
        toOutput->removeInput(g, junction);
    }
}

void AudioNodeInput::disconnectAll(ContextGraphLock & g, std::shared_ptr<AudioNodeInput> fromInput)
{
    ASSERT(g.context());
    if (!fromInput || !fromInput->destinationNode())
        return;

    for (auto i : fromInput->m_connectedOutputs)
    {
        auto o = i.lock();
        if (o)
        {
            fromInput->junctionDisconnectOutput(o);
            o->removeInput(g, fromInput);
        }
    }
}

void AudioNodeInput::updateInternalBus(ContextRenderLock & r)
{
    int numberOfInputChannels = numberOfChannels(r);

    if (numberOfInputChannels == m_internalSummingBus->numberOfChannels())
        return;

    m_internalSummingBus = std::unique_ptr<AudioBus>(new AudioBus(numberOfInputChannels, AudioNode::ProcessingSizeInFrames));
}

int AudioNodeInput::numberOfChannels(ContextRenderLock & r) const
{
    ChannelCountMode mode = destinationNode()->channelCountMode();

    if (mode == ChannelCountMode::Explicit)
    {
        return destinationNode()->channelCount();
    }

    // Find the number of channels of the connection with the largest number of channels.
    int maxChannels = 1;  // one channel is the minimum allowed

    int c = numberOfRenderingConnections(r);
    for (int i = 0; i < c; ++i)
    {
        auto output = renderingOutput(r, i);
        if (output)
        {
            int c = output->bus(r)->numberOfChannels();
            if (c > maxChannels)
                maxChannels = c;
        }
    }

    if (mode == ChannelCountMode::ClampedMax)
    {
        maxChannels = min(maxChannels, destinationNode()->channelCount());
    }

    return maxChannels;
}

AudioBus * AudioNodeInput::bus(ContextRenderLock & r)
{
    // Handle single connection specially to allow for in-place processing.
    // note: The webkit sources check for max, but I can't see how that's correct

    // @tofix - did I miss part of the merge?
    if (numberOfRenderingConnections(r) == 1)  // && node()->channelCountMode() == ChannelCountMode::Max)
    {
        std::shared_ptr<AudioNodeOutput> output = renderingOutput(r, 0);
        if (output)
        {
            return output->bus(r);
        }
    }

    // Multiple connections case (or no connections).
    return m_internalSummingBus.get();
}

AudioBus * AudioNodeInput::pull(ContextRenderLock & r, AudioBus * inPlaceBus, int bufferSize)
{
    updateRenderingState(r);
    m_destinationNode->checkNumberOfChannelsForInput(r, this);

    size_t num_connections = numberOfRenderingConnections(r);

    // Handle single connection case.
    if (num_connections == 1)
    {
        // If this input is simply passing data through, then immediately delegate the pull request to it.
        auto output = renderingOutput(r, 0);
        if (output)
        {
            return output->pull(r, inPlaceBus, bufferSize);
        }

        num_connections = 0;  // if there's a single input, but it has no output; treat this input as silent.
    }

    if (num_connections == 0)
    {
        // Generate silence if we're not connected to anything, and return the silent bus
        /// @TODO a possible optimization is to flag silence and propagate it to consumers of this input.
        m_internalSummingBus->zero();
        return m_internalSummingBus.get();
    }

    // multiple connections
    m_internalSummingBus->zero();

    for (int i = 0; i < num_connections; ++i)
    {
        auto output = renderingOutput(r, i);
        if (output)
        {
            // Render audio from this output.
            AudioBus * connectionBus = output->pull(r, 0, bufferSize);

            // Sum, with unity-gain.
            m_internalSummingBus->sumFrom(*connectionBus);
        }
    }
    return m_internalSummingBus.get();
}

}  // namespace lab
