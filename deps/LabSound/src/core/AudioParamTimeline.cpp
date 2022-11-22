// License: BSD 2 Clause
// Copyright (C) 2011, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/AudioParamTimeline.h"
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioNode.h"
#include "LabSound/core/Macros.h"

#include "LabSound/extended/AudioContextLock.h"

#include "internal/Assertions.h"
#include "internal/AudioUtilities.h"

#include <algorithm>

using namespace std;

namespace lab
{

//@tofix - is there any reason this should be per object instead of static?
namespace
{
    std::mutex m_eventsMutex;
}

void AudioParamTimeline::setValueAtTime(float value, float time)
{
    insertEvent(ParamEvent(ParamEvent::SetValue, value, time, 0, 0, {}));
}

void AudioParamTimeline::linearRampToValueAtTime(float value, float time)
{
    insertEvent(ParamEvent(ParamEvent::LinearRampToValue, value, time, 0, 0, {}));
}

void AudioParamTimeline::exponentialRampToValueAtTime(float value, float time)
{
    insertEvent(ParamEvent(ParamEvent::ExponentialRampToValue, value, time, 0, 0, {}));
}

void AudioParamTimeline::setTargetAtTime(float target, float time, float timeConstant)
{
    insertEvent(ParamEvent(ParamEvent::SetTarget, target, time, timeConstant, 0, {}));
}

void AudioParamTimeline::setValueCurveAtTime(std::vector<float> & curve, float time, float duration)
{
    insertEvent(ParamEvent(ParamEvent::SetValueCurve, 0, time, 0, duration, curve));
}

static bool isValidNumber(float x)
{
    return !std::isnan(x) && !std::isinf(x);
}

void AudioParamTimeline::insertEvent(const ParamEvent & event)
{
    // Sanity check the event. Be super careful we're not getting infected with NaN or Inf.
    bool isValid = event.type() < ParamEvent::LastType
                   && isValidNumber(event.value())
                   && isValidNumber(event.time())
                   && isValidNumber(event.timeConstant())
                   && isValidNumber(event.duration())
                   && event.duration() >= 0;

    ASSERT(isValid);

    if (!isValid)
        return;

    std::lock_guard<std::mutex> lock(m_eventsMutex);

    unsigned i = 0;
    float insertTime = event.time();

    for (i = 0; i < m_events.size(); ++i)
    {

        if (event.type() == ParamEvent::SetValueCurve)
        {
            // If this event is a SetValueCurve, make sure it doesn't overlap any existing
            // event. It's ok if the SetValueCurve starts at the same time as the end of some other
            // duration.
            double endTime = event.time() + event.duration();
            if (m_events[i].time() > event.time() && m_events[i].time() < endTime)
            {
                throw std::runtime_error("ParamEvent::SetValueCurve overlaps existing");
            }
        }
        else
        {
            // Otherwise, make sure this event doesn't overlap any existing SetValueCurve event.
            if (m_events[i].type() == ParamEvent::SetValueCurve)
            {
                double endTime = m_events[i].time() + m_events[i].duration();
                if (event.time() >= m_events[i].time() && event.time() < endTime)
                {
                    throw std::runtime_error("ParamEvent::SetValueCurve overlaps existing");
                }
            }
        }

        // Overwrite same event type and time.
        if (m_events[i].time() == insertTime && m_events[i].type() == event.type())
        {
            m_events[i] = event;
            return;
        }

        if (m_events[i].time() > insertTime)
        {
            break;
        }
    }

    m_events.insert(m_events.begin() + i, event);
}

void AudioParamTimeline::cancelScheduledValues(float startTime)
{
    std::lock_guard<std::mutex> lock(m_eventsMutex);

    // Remove all events starting at startTime.
    for (unsigned i = 0; i < m_events.size(); ++i)
    {
        if (m_events[i].time() >= startTime)
        {
            m_events.erase(m_events.begin() + i, m_events.end());
            break;
        }
    }
}

float AudioParamTimeline::valueForContextTime(ContextRenderLock & r, float defaultValue, bool & hasValue)
{
    auto context = r.context();
    if (!context)
        return defaultValue;

    std::unique_lock<std::mutex> lock(m_eventsMutex, std::try_to_lock);
    if (!lock.owns_lock() || !m_events.size() || context->currentTime() < m_events[0].time())
    {
        hasValue = false;
        return defaultValue;
    }

    // Ask for just a single value.
    double sampleRate = context->sampleRate();
    double startTime = context->currentTime();
    double endTime = startTime + 1.1 / sampleRate;  // time just beyond one sample-frame
    double controlRate = sampleRate / AudioNode::ProcessingSizeInFrames;  // one parameter change per render quantum
    float value = valuesForTimeRange(startTime, endTime, defaultValue, &value, 1, sampleRate, controlRate);

    hasValue = true;
    return value;
}

float AudioParamTimeline::valuesForTimeRange(
    double startTime,
    double endTime,
    float defaultValue,
    float * values,
    size_t numberOfValues,
    double sampleRate,
    double controlRate)
{
    float value = valuesForTimeRangeImpl(startTime, endTime, defaultValue, values, numberOfValues, sampleRate, controlRate);
    return value;
}

float AudioParamTimeline::valuesForTimeRangeImpl(
    double startTime,
    double endTime,
    float defaultValue,
    float * values,
    size_t numberOfValues,
    double sampleRate,
    double controlRate)
{
    if (!values)
        return defaultValue;

    // Return default value if there are no events matching the desired time range.
    std::unique_lock<std::mutex> lock(m_eventsMutex, std::try_to_lock);
    if (!lock.owns_lock() || !m_events.size() || endTime <= m_events[0].time())
    {
        for (unsigned i = 0; i < numberOfValues; ++i)
            values[i] = defaultValue;
        return defaultValue;
    }

    // Maintain a running time and index for writing the values buffer.
    double currentTime = startTime;
    unsigned writeIndex = 0;

    // If first event is after startTime then fill initial part of values buffer with defaultValue
    // until we reach the first event time.
    double firstEventTime = m_events[0].time();
    if (firstEventTime > startTime)
    {
        double fillToTime = std::min(endTime, firstEventTime);
        size_t fillToFrame = AudioUtilities::timeToSampleFrame(fillToTime - startTime, sampleRate);
        fillToFrame = std::min(fillToFrame, numberOfValues);
        for (; writeIndex < fillToFrame; ++writeIndex)
            values[writeIndex] = defaultValue;

        currentTime = fillToTime;
    }

    float value = defaultValue;

    // Go through each event and render the value buffer where the times overlap,
    // stopping when we've rendered all the requested values.
    // FIXME: could try to optimize by avoiding having to iterate starting from the very first event
    // and keeping track of a "current" event index.
    int n = static_cast<int>(m_events.size());
    for (int i = 0; i < n && writeIndex < numberOfValues; ++i)
    {
        ParamEvent & event = m_events[i];
        ParamEvent * nextEvent = i < n - 1 ? &(m_events[i + 1]) : 0;

        // Wait until we get a more recent event.
        if (nextEvent && nextEvent->time() < currentTime)
            continue;

        float value1 = event.value();
        double time1 = event.time();
        float value2 = nextEvent ? nextEvent->value() : value1;
        double time2 = nextEvent ? nextEvent->time() : endTime + 1;

        double deltaTime = time2 - time1;
        float k = static_cast<float>(deltaTime > 0 ? 1 / deltaTime : 0);
        double sampleFrameTimeIncr = 1 / sampleRate;

        double fillToTime = std::min(endTime, time2);
        size_t fillToFrame = AudioUtilities::timeToSampleFrame(fillToTime - startTime, sampleRate);
        fillToFrame = std::min(fillToFrame, numberOfValues);

        ParamEvent::Type nextEventType = nextEvent ? static_cast<ParamEvent::Type>(nextEvent->type()) : ParamEvent::LastType /* unknown */;

        // First handle linear and exponential ramps which require looking ahead to the next event.
        if (nextEventType == ParamEvent::LinearRampToValue)
        {
            for (; writeIndex < fillToFrame; ++writeIndex)
            {
                float x = static_cast<float>(currentTime - time1) * k;
                value = (1 - x) * value1 + x * value2;
                values[writeIndex] = value;
                currentTime += sampleFrameTimeIncr;
            }
        }
        else if (nextEventType == ParamEvent::ExponentialRampToValue)
        {
            if (value1 <= 0 || value2 <= 0)
            {
                // Handle negative values error case by propagating previous value.
                for (; writeIndex < fillToFrame; ++writeIndex)
                    values[writeIndex] = value;
            }
            else
            {
                float numSampleFrames = static_cast<float>(deltaTime * sampleRate);
                // The value goes exponentially from value1 to value2 in a duration of deltaTime seconds (corresponding to numSampleFrames).
                // Compute the per-sample multiplier.
                float multiplier = powf(value2 / value1, 1 / numSampleFrames);

                // Set the starting value of the exponential ramp. This is the same as multiplier ^
                // AudioUtilities::timeToSampleFrame(currentTime - time1, sampleRate), but is more
                // accurate, especially if multiplier is close to 1.
                value = value1 * powf(value2 / value1, AudioUtilities::timeToSampleFrame(currentTime - time1, sampleRate) / numSampleFrames);

                for (; writeIndex < fillToFrame; ++writeIndex)
                {
                    values[writeIndex] = value;
                    value *= multiplier;
                    currentTime += sampleFrameTimeIncr;
                }
            }
        }
        else
        {
            // Handle event types not requiring looking ahead to the next event.
            switch (event.type())
            {
                case ParamEvent::SetValue:
                case ParamEvent::LinearRampToValue:
                case ParamEvent::ExponentialRampToValue:
                {
                    currentTime = fillToTime;

                    // Simply stay at a constant value.
                    value = event.value();
                    for (; writeIndex < fillToFrame; ++writeIndex)
                        values[writeIndex] = value;

                    break;
                }

                case ParamEvent::SetTarget:
                {
                    currentTime = fillToTime;

                    // Exponential approach to target value with given time constant.
                    float target = event.value();
                    float timeConstant = event.timeConstant();
                    float discreteTimeConstant = static_cast<float>(AudioUtilities::discreteTimeConstantForSampleRate(timeConstant, controlRate));

                    for (; writeIndex < fillToFrame; ++writeIndex)
                    {
                        values[writeIndex] = value;
                        value += (target - value) * discreteTimeConstant;
                    }

                    break;
                }

                case ParamEvent::SetValueCurve:
                {
                    std::vector<float> & curve = event.curve();
                    float * curveData = curve.size() > 0 ? curve.data() : 0;
                    size_t numberOfCurvePoints = curve.size() > 0 ? curve.size() : 0;

                    // Curve events have duration, so don't just use next event time.
                    float duration = event.duration();

                    // How much to step the curve index for each frame.  This is basically the term
                    // (N - 1)/Td in the specification.
                    float curvePointsPerFrame = static_cast<float>((numberOfCurvePoints - 1) / duration / sampleRate);

                    if (curve.size() == 0 || !curveData || !numberOfCurvePoints || duration <= 0 || sampleRate <= 0)
                    {
                        // Error condition - simply propagate previous value.
                        currentTime = fillToTime;
                        for (; writeIndex < fillToFrame; ++writeIndex)
                            values[writeIndex] = value;
                        break;
                    }

                    // Save old values and recalculate information based on the curve's duration
                    // instead of the next event time.
                    size_t nextEventFillToFrame = fillToFrame;
                    double nextEventFillToTime = fillToTime;
                    fillToTime = std::min(endTime, time1 + duration);

                    // |fillToTime| can be greater than |startTime| when the end of the
                    // setValueCurve automation has been reached, but the next automation has not
                    // yet started. In this case, |fillToTime| is clipped to |time1|+|duration|
                    // above, but |startTime| will keep increasing (because the current time is
                    // increasing).
                    fillToFrame = AudioUtilities::timeToSampleFrame(std::max(0.0, fillToTime - startTime), sampleRate);

                    fillToFrame = std::min(fillToFrame, numberOfValues);

                    // Index into the curve data using a floating-point value.
                    // We're scaling the number of curve points by the duration (see curvePointsPerFrame).
                    float curveVirtualIndex = 0;
                    if (time1 < currentTime)
                    {
                        // Index somewhere in the middle of the curve data.
                        // Don't use timeToSampleFrame() since we want the exact floating-point frame.
                        float frameOffset = static_cast<float>((currentTime - time1) * sampleRate);
                        curveVirtualIndex = curvePointsPerFrame * frameOffset;
                    }

                    // Render the stretched curve data using nearest neighbor sampling.
                    // Oversampled curve data can be provided if smoothness is desired.
                    for (; writeIndex < fillToFrame; ++writeIndex)
                    {
                        // Ideally we'd use round() from MathExtras, but we're in a tight loop here
                        // and we're trading off precision for extra speed.
                        unsigned curveIndex = static_cast<unsigned>(0.5 + curveVirtualIndex);

                        curveVirtualIndex += curvePointsPerFrame;

                        // Bounds check.
                        if (curveIndex < numberOfCurvePoints)
                            value = curveData[curveIndex];

                        values[writeIndex] = value;
                    }

                    // If there's any time left after the duration of this event and the start
                    // of the next, then just propagate the last value.
                    for (; writeIndex < nextEventFillToFrame; ++writeIndex)
                        values[writeIndex] = value;

                    // Re-adjust current time
                    currentTime = nextEventFillToTime;

                    break;
                }
            }
        }
    }

    // If there's any time left after processing the last event then just propagate the last value
    // to the end of the values buffer.
    for (; writeIndex < numberOfValues; ++writeIndex)
        values[writeIndex] = value;

    return value;
}

}  // namespace lab
