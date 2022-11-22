// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef AudioDSPKernelProcessor_h
#define AudioDSPKernelProcessor_h

#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioProcessor.h"

#include "internal/AudioDSPKernel.h"

#include <vector>

namespace lab
{

class AudioBus;
class AudioProcessor;

// AudioDSPKernelProcessor processes one input -> one output (N channels each)
// It uses one AudioDSPKernel object per channel to do the processing, thus there is no cross-channel processing.
// Despite this limitation it turns out to be a very common and useful type of processor.

class AudioDSPKernelProcessor : public AudioProcessor
{
public:
    // numberOfChannels may be later changed if object is not yet in an "initialized" state
    AudioDSPKernelProcessor();
    virtual ~AudioDSPKernelProcessor()
    {
        if (isInitialized()) uninitialize();
    }

    // Subclasses create the appropriate type of processing kernel here.
    // We'll call this to create a kernel for each channel.
    virtual AudioDSPKernel * createKernel() = 0;

    // AudioProcessor methods
    virtual void initialize() override;
    virtual void uninitialize() override;
    virtual void process(ContextRenderLock &, const AudioBus * source, AudioBus * destination, int framesToProcess) override;
    virtual void reset() override;

    virtual double tailTime(ContextRenderLock & r) const override;
    virtual double latencyTime(ContextRenderLock & r) const override;

protected:
    std::vector<std::unique_ptr<AudioDSPKernel>> m_kernels;
    bool m_hasJustReset;
};

}  // namespace lab

#endif  // AudioDSPKernelProcessor_h
