// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef AudioProcessor_h
#define AudioProcessor_h

#include <stddef.h>

namespace lab
{

class AudioBus;
class ContextRenderLock;

// AudioProcessor is an abstract base class representing an audio signal processing object with a single input and a single output,
// where the number of input channels equals the number of output channels. It can be used as one part of a complex DSP algorithm,
// or as the processor for a basic (one input - one output) AudioNode.
class AudioProcessor
{

public:
    AudioProcessor()
    {
    }
    virtual ~AudioProcessor() {}

    // Full initialization can be done here instead of in the constructor.
    virtual void initialize() = 0;
    virtual void uninitialize() = 0;

    // Processes the source to destination bus.  The number of channels must match in source and destination.
    virtual void process(ContextRenderLock &, const AudioBus * source, AudioBus * destination, int bufferSize) = 0;

    // Resets filter state
    virtual void reset() = 0;
    bool isInitialized() const { return m_initialized; }

    virtual double tailTime(ContextRenderLock & r) const = 0;
    virtual double latencyTime(ContextRenderLock & r) const = 0;

protected:
    bool m_initialized = false;
};

}  // namespace lab

#endif  // AudioProcessor_h
