// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/AudioProcessor.h"

#include "LabSound/extended/PWMNode.h"
#include "LabSound/extended/Registry.h"

#include <iostream>

using namespace lab;

namespace lab
{

////////////////////////////////////
// Private PWMNode Implementation //
////////////////////////////////////

class PWMNode::PWMNodeInternal : public lab::AudioProcessor
{

public:
    PWMNodeInternal()
        : AudioProcessor()
    {
    }

    virtual ~PWMNodeInternal() {}

    virtual void initialize() override {}

    virtual void uninitialize() override {}

    // Processes the source to destination bus.
    virtual void process(ContextRenderLock &,
                         const lab::AudioBus * source, lab::AudioBus * destination,
                         int framesToProcess) override
    {
        if (!source->numberOfChannels() || !destination->numberOfChannels())
            return;
            
        const float * carrierP = source->channelByType(Channel::Left)->data();
        const float * modP = source->channelByType(Channel::Right)->data();

        if (!modP && carrierP)
        {
            destination->copyFrom(*source);
        }
        else if (modP && carrierP)
        {
            float * destP = destination->channel(0)->mutableData();
            int n = framesToProcess;
            while (n--)
            {
                float carrier = *carrierP++;
                float mod = *modP++;
                *destP++ = (carrier > mod) ? 1.0f : -1.0f;
            }
        }
    }

    virtual void reset() override {}

    virtual double tailTime(ContextRenderLock & r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock & r) const override { return 0; }
};

////////////////////
// Public PWMNode //
////////////////////

PWMNode::PWMNode(AudioContext & ac)
    : lab::AudioBasicProcessorNode(ac)
{
    addInput(std::unique_ptr<AudioNodeInput>(new AudioNodeInput(this)));
    m_processor.reset(new PWMNodeInternal());
    internalNode = static_cast<PWMNodeInternal *>(m_processor.get());  // Currently unused
    initialize();
}

PWMNode::~PWMNode()
{
    uninitialize();
}
}
