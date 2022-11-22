// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/ChannelMergerNode.h"
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/extended/Registry.h"

#include "internal/Assertions.h"

using namespace std;

namespace lab
{

ChannelMergerNode::ChannelMergerNode(AudioContext& ac, int numberOfInputs_)
    : AudioNode(ac), m_desiredNumberOfOutputChannels(1)
{
    addInputs(numberOfInputs_);
    addOutput(std::unique_ptr<AudioNodeOutput>(new AudioNodeOutput(this, 1)));
    initialize();
}

void ChannelMergerNode::addInputs(int n)
{
    // Create the requested number of inputs.
    for (int i = 0; i < n; ++i)
        addInput(std::unique_ptr<AudioNodeInput>(new AudioNodeInput(this)));
}

void ChannelMergerNode::process(ContextRenderLock & r, int bufferSize)
{
    auto output = this->output(0);
    ASSERT_UNUSED(bufferSize, bufferSize == output->bus(r)->length());

    if (m_desiredNumberOfOutputChannels != output->numberOfChannels())
    {
        output->setNumberOfChannels(r, m_desiredNumberOfOutputChannels);
    }

    // Merge all the channels from all the inputs into one output.
    uint32_t outputChannelIndex = 0;
    for (int i = 0; i < numberOfInputs(); ++i)
    {
        auto input = this->input(i);

        if (input->isConnected())
        {
            size_t numberOfInputChannels = input->bus(r)->numberOfChannels();

            // Merge channels from this particular input.
            for (int j = 0; j < numberOfInputChannels; ++j)
            {
                AudioChannel * inputChannel = input->bus(r)->channel(j);
                AudioChannel * outputChannel = output->bus(r)->channel(outputChannelIndex);

                outputChannel->copyFrom(inputChannel);
                ++outputChannelIndex;
            }
        }
    }

    ASSERT(outputChannelIndex == output->numberOfChannels());
}


}  // namespace lab
