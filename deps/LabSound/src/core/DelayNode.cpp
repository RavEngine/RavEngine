// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/DelayNode.h"
#include "LabSound/core/AudioBasicProcessorNode.h"
#include "LabSound/core/AudioProcessor.h"
#include "LabSound/extended/Registry.h"

#include "internal/AudioDSPKernel.h"
#include "internal/DelayProcessor.h"

namespace lab
{

DelayNode::DelayNode(AudioContext& ac, double maxDelayTime)
    : AudioBasicProcessorNode(ac)
{
    if (maxDelayTime < 0)
        maxDelayTime = 0;  // delay node can't predict the future

    m_processor = std::make_unique<DelayProcessor>(ac.sampleRate(), maxDelayTime);

    m_settings.push_back(delayProcessor()->delayTime());

    initialize();
}

std::shared_ptr<AudioSetting> DelayNode::delayTime()
{
    return delayProcessor()->delayTime();
}

DelayProcessor * DelayNode::delayProcessor()
{
    return static_cast<DelayProcessor *>(processor());
}

}  // lab
