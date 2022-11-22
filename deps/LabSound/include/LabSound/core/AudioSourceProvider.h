// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef AudioSourceProvider_h
#define AudioSourceProvider_h

#include "LabSound/extended/AudioContextLock.h"
#include "LabSound/core/AudioBus.h"

namespace lab
{
class AudioBus;

/////////////////////////////
//   AudioSourceProvider   //
/////////////////////////////

// Abstract base-class for a pull-model client.
// provideInput() gets called repeatedly to render time-slices of a continuous audio stream.
struct AudioSourceProvider
{
    virtual void provideInput(AudioBus * bus, int bufferSize) = 0;
    virtual ~AudioSourceProvider() {}
};

////////////////////////////
//   AudioHardwareInput   //
////////////////////////////

// AudioHardwareInput allows us to expose an AudioSourceProvider for local/live audio input.
// If there is local/live audio input, we call set() with the audio input data every render quantum.
// `set()` is called in ... which is one or two frames above the actual hardware io.
class AudioHardwareInput : public AudioSourceProvider
{
    AudioBus m_sourceBus;

public:
    AudioHardwareInput(int channelCount)
        : m_sourceBus(channelCount, AudioNode::ProcessingSizeInFrames)
    {
    }

    virtual ~AudioHardwareInput() {}

    void set(AudioBus * bus)
    {
        if (bus) m_sourceBus.copyFrom(*bus);
    }

    // Satisfy the AudioSourceProvider interface
    virtual void provideInput(AudioBus * destinationBus, int bufferSize)
    {
        bool isGood = destinationBus && destinationBus->length() == bufferSize && m_sourceBus.length() == bufferSize;
        //ASSERT(isGood);
        if (isGood) destinationBus->copyFrom(m_sourceBus);
    }
};

}  // lab

#endif  // AudioSourceProvider_h
