// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/AudioNode.h"

#include "internal/Assertions.h"
#include "internal/AudioUtilities.h"
#include "internal/DelayDSPKernel.h"

#include <algorithm>

using namespace std;

namespace lab
{

const float SmoothingTimeConstant = 0.020f;  // 20ms

DelayDSPKernel::DelayDSPKernel(DelayProcessor * processor, float sampleRate)
    : AudioDSPKernel(processor)
    , m_writeIndex(0)
    , m_firstTime(true)
    , m_delayTimes(AudioNode::ProcessingSizeInFrames)
{
    ASSERT(processor);
    if (!processor)
        return;

    m_maxDelayTime = processor->maxDelayTime();
    ASSERT(m_maxDelayTime >= 0);
    if (m_maxDelayTime < 0)
        return;

    m_buffer.allocate(bufferLengthForDelay(m_maxDelayTime, sampleRate));
    m_buffer.zero();

    m_smoothingRate = AudioUtilities::discreteTimeConstantForSampleRate(SmoothingTimeConstant, sampleRate);
}

DelayDSPKernel::DelayDSPKernel(double maxDelayTime, float sampleRate)
    : AudioDSPKernel()
    , m_maxDelayTime(maxDelayTime)
    , m_writeIndex(0)
    , m_firstTime(true)
{
    ASSERT(maxDelayTime > 0.0);
    if (maxDelayTime <= 0.0)
        return;

    size_t bufferLength = bufferLengthForDelay(maxDelayTime, sampleRate);
    ASSERT(bufferLength);
    if (!bufferLength)
        return;

    m_buffer.allocate(bufferLength);
    m_buffer.zero();

    m_smoothingRate = AudioUtilities::discreteTimeConstantForSampleRate(SmoothingTimeConstant, sampleRate);
}

size_t DelayDSPKernel::bufferLengthForDelay(double maxDelayTime, double sampleRate) const
{
    // Compute the length of the buffer needed to handle a max delay of |maxDelayTime|. One is
    // added to handle the case where the actual delay equals the maximum delay.
    return 1 + AudioUtilities::timeToSampleFrame(maxDelayTime, sampleRate);
}

void DelayDSPKernel::process(ContextRenderLock & r, const float * source, float * destination, int framesToProcess)
{
    size_t bufferLength = m_buffer.size();
    float * buffer = m_buffer.data();

    ASSERT(bufferLength);
    if (!bufferLength)
        return;

    ASSERT(source && destination);
    if (!source || !destination)
        return;

    float sampleRate = r.context()->sampleRate();
    double delayTime = 0;
    float * delayTimes = m_delayTimes.data();
    double maxTime = maxDelayTime();

#if 1
    /// @TODO before deleting the else clause here, is there a legitimate reason to have the delayTime be automated?
    /// is it not just a setting? If it's actually an audio rate signal, then delayTime should be switched back
    /// from AudioSetting to AudioParam.
    delayTime = delayProcessor() ? delayProcessor()->delayTime()->valueFloat() : m_desiredDelayFrames / sampleRate;

    // Make sure the delay time is in a valid range.
    delayTime = min(maxTime, delayTime);
    delayTime = max(0.0, delayTime);

    if (m_firstTime)
    {
        m_currentDelayTime = delayTime;
        m_firstTime = false;
    }
    bool sampleAccurate = false;
#else
    bool sampleAccurate = delayProcessor() && delayProcessor()->delayTime()->hasSampleAccurateValues();

    if (sampleAccurate)
        delayProcessor()->delayTime()->calculateSampleAccurateValues(r, delayTimes, framesToProcess);
    else
    {
        delayTime = delayProcessor() ? delayProcessor()->delayTime()->finalValue(r) : m_desiredDelayFrames / sampleRate;

        // Make sure the delay time is in a valid range.
        delayTime = min(maxTime, delayTime);
        delayTime = max(0.0, delayTime);

        if (m_firstTime)
        {
            m_currentDelayTime = delayTime;
            m_firstTime = false;
        }
    }
#endif

    for (int i = 0; i < framesToProcess; ++i)
    {
        if (sampleAccurate)
        {
            delayTime = delayTimes[i];
            delayTime = std::min(maxTime, delayTime);
            delayTime = std::max(0.0, delayTime);
            m_currentDelayTime = delayTime;
        }
        else
        {
            // Approach desired delay time.
            m_currentDelayTime += (delayTime - m_currentDelayTime) * m_smoothingRate;
        }

        double desiredDelayFrames = m_currentDelayTime * sampleRate;

        double readPosition = m_writeIndex + bufferLength - desiredDelayFrames;
        if (readPosition >= bufferLength)
            readPosition -= bufferLength;

        // Linearly interpolate in-between delay times.
        int readIndex1 = static_cast<int>(readPosition);
        int readIndex2 = (readIndex1 + 1) % bufferLength;
        double interpolationFactor = readPosition - readIndex1;

        double input = static_cast<float>(*source++);
        buffer[m_writeIndex] = static_cast<float>(input);
        m_writeIndex = (m_writeIndex + 1) % bufferLength;

        double sample1 = buffer[readIndex1];
        double sample2 = buffer[readIndex2];

        double output = (1.0 - interpolationFactor) * sample1 + interpolationFactor * sample2;

        *destination++ = static_cast<float>(output);
    }
}

void DelayDSPKernel::reset()
{
    m_firstTime = true;
    m_buffer.zero();
}

double DelayDSPKernel::tailTime(ContextRenderLock & r) const
{
    return m_maxDelayTime;
}

double DelayDSPKernel::latencyTime(ContextRenderLock & r) const
{
    return 0;
}

}  // namespace lab
