// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef AudioDSPKernel_h
#define AudioDSPKernel_h

#include "LabSound/extended/AudioContextLock.h"

namespace lab
{

class AudioDSPKernelProcessor;

// AudioDSPKernel does the processing for one channel of an AudioDSPKernelProcessor.
class AudioDSPKernel
{
public:

    AudioDSPKernel(AudioDSPKernelProcessor * kernelProcessor) : m_kernelProcessor(kernelProcessor) {}
    AudioDSPKernel() = default;

    virtual ~AudioDSPKernel() {}

    // Subclasses must override process() to do the processing and reset() to reset DSP state.
    virtual void process(ContextRenderLock &, const float * source, float * destination, int framesToProcess) = 0;
    virtual void reset() = 0;

    AudioDSPKernelProcessor * processor() { return m_kernelProcessor; }
    const AudioDSPKernelProcessor * processor() const { return m_kernelProcessor; }

    virtual double tailTime(ContextRenderLock & r) const = 0;
    virtual double latencyTime(ContextRenderLock & r) const = 0;

protected:
    AudioDSPKernelProcessor * m_kernelProcessor {nullptr};
};

}  // namespace lab

#endif  // AudioDSPKernel_h

