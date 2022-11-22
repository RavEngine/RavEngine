// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef DelayProcessor_h
#define DelayProcessor_h

#include "LabSound/core/AudioSetting.h"

#include "internal/AudioDSPKernelProcessor.h"

namespace lab
{

class AudioDSPKernel;

class DelayProcessor : public AudioDSPKernelProcessor
{
    std::shared_ptr<AudioSetting> m_delayTime;
    double m_maxDelayTime;
    float m_sampleRate;

public:
    DelayProcessor(float sampleRate, double maxDelayTime);

    virtual ~DelayProcessor();

    virtual AudioDSPKernel * createKernel();

    std::shared_ptr<AudioSetting> delayTime() const { return m_delayTime; }

    double maxDelayTime() { return m_maxDelayTime; }
};

}  // namespace lab

#endif  // DelayProcessor_h
