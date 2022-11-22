// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2012, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/AudioHardwareInputNode.h"
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/AudioSourceProvider.h"

#include "LabSound/extended/AudioContextLock.h"
#include "LabSound/extended/Logging.h"
#include "LabSound/extended/Registry.h"

#include <memory>

namespace lab
{

AudioHardwareInputNode::AudioHardwareInputNode(AudioContext & ac, AudioSourceProvider * audioSourceProvider)
    : AudioNode(ac)
    , m_audioSourceProvider(audioSourceProvider)
{
    addOutput(std::unique_ptr<AudioNodeOutput>(new AudioNodeOutput(this, 1)));  // Num output channels will be re-configured in process
    initialize();
}


AudioHardwareInputNode::~AudioHardwareInputNode()
{
    uninitialize();
}

void AudioHardwareInputNode::process(ContextRenderLock &r, int bufferSize)
{
    AudioBus * outputBus = output(0)->bus(r);

    // This used to be the function of a manual call to setFormat()
    if (m_sourceNumberOfChannels == 0)
    {
        auto DeviceAsAudioNode = r.context()->device();

        if (auto DeviceAsRenderCallback = std::dynamic_pointer_cast<AudioDeviceRenderCallback>(DeviceAsAudioNode))
        {
            auto inputConfig = DeviceAsRenderCallback->getInputConfig();
            m_sourceNumberOfChannels = inputConfig.desired_channels;
            output(0)->setNumberOfChannels(r, m_sourceNumberOfChannels);  // Reconfigure the output's number of channels.
            outputBus = output(0)->bus(r);  // outputBus pointer was invalidated
        }
    }

    if (!m_audioSourceProvider)
    {
        outputBus->zero();
        return;
    }

    if (m_sourceNumberOfChannels != outputBus->numberOfChannels())
    {
        outputBus->zero();
        return;
    }

    // Use a tryLock() to avoid contention in the real-time audio thread.
    // If we fail to acquire the lock then the MediaStream must be in the middle of
    // a format change, so we output silence in this case.
    if (r.context())
    {
        // provide entire buffer
        m_audioSourceProvider->provideInput(outputBus, bufferSize);
    }
    else
    {
        // We failed to acquire the lock.
        outputBus->zero();
    }
}

void AudioHardwareInputNode::reset(ContextRenderLock & r)
{
}

}  // namespace lab
