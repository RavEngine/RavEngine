// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef DelayDSPKernel_h
#define DelayDSPKernel_h

#include "LabSound/core/AudioArray.h"

#include "internal/AudioDSPKernel.h"
#include "internal/DelayProcessor.h"

namespace lab
{

class DelayProcessor;

class DelayDSPKernel : public AudioDSPKernel
{
public:
    explicit DelayDSPKernel(DelayProcessor *, float sampleRate);
    DelayDSPKernel(double maxDelayTime, float sampleRate);
    virtual ~DelayDSPKernel() {}

    virtual void process(ContextRenderLock &, const float * source, float * destination, int framesToProcess) override;
    virtual void reset() override;

    double maxDelayTime() const { return m_maxDelayTime; }

    void setDelayFrames(double numberOfFrames) { m_desiredDelayFrames = numberOfFrames; }

    virtual double tailTime(ContextRenderLock & r) const override;
    virtual double latencyTime(ContextRenderLock & r) const override;

private:
    AudioFloatArray m_buffer;
    double m_maxDelayTime;
    int m_writeIndex;
    double m_currentDelayTime;
    double m_smoothingRate;
    bool m_firstTime;
    double m_desiredDelayFrames;

    AudioFloatArray m_delayTimes;

    DelayProcessor * delayProcessor() { return static_cast<DelayProcessor *>(processor()); }
    size_t bufferLengthForDelay(double delayTime, double sampleRate) const;
};

}  // namespace lab

#endif  // DelayDSPKernel_h
