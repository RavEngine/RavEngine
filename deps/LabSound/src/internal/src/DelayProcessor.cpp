// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "internal/DelayProcessor.h"
#include "internal/AudioDSPKernelProcessor.h"
#include "internal/DelayDSPKernel.h"

using namespace std;

namespace lab
{

DelayProcessor::DelayProcessor(float sampleRate, double maxDelayTime)
    : AudioDSPKernelProcessor()
    , m_maxDelayTime(maxDelayTime)
    , m_sampleRate(sampleRate)
{
    m_delayTime = std::make_shared<AudioSetting>("delayTime", "DELY", AudioSetting::Type::Float);
}

DelayProcessor::~DelayProcessor()
{
    if (isInitialized())
        uninitialize();
}

AudioDSPKernel * DelayProcessor::createKernel()
{
    return new DelayDSPKernel(this, m_sampleRate);
}

}  // namespace lab
