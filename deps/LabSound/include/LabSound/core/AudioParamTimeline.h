// License: BSD 3 Clause
// Copyright (C) 2011, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef AudioParamTimeline_h
#define AudioParamTimeline_h

#include "LabSound/core/AudioContext.h"
#include <mutex>
#include <vector>

namespace lab
{

class AudioParamTimeline
{

public:
    AudioParamTimeline() {}

    void setValueAtTime(float value, float time);
    void linearRampToValueAtTime(float value, float time);
    void exponentialRampToValueAtTime(float value, float time);
    void setTargetAtTime(float target, float time, float timeConstant);
    void setValueCurveAtTime(std::vector<float> & curve, float time, float duration);
    void cancelScheduledValues(float startTime);

    // hasValue is set to true if a valid timeline value is returned.
    // otherwise defaultValue is returned.
    float valueForContextTime(ContextRenderLock &, float defaultValue, bool & hasValue);

    // Given the time range, calculates parameter values into the values buffer
    // and returns the last parameter value calculated for "values" or the defaultValue if none were calculated.
    // controlRate is the rate (number per second) at which parameter values will be calculated.
    // It should equal sampleRate for sample-accurate parameter changes, and otherwise will usually match
    // the render quantum size such that the parameter value changes once per render quantum.
    float valuesForTimeRange(double startTime, double endTime, float defaultValue,
                             float * values, size_t numberOfValues, double sampleRate, double controlRate);

    bool hasValues() { return m_events.size() > 0; }

private:

    // @tofix - move to implementation file to hide from public API

    class ParamEvent
    {

    public:
        enum Type
        {
            SetValue,
            LinearRampToValue,
            ExponentialRampToValue,
            SetTarget,
            SetValueCurve,
            LastType
        };

        ParamEvent(Type type, float value, float time, float timeConstant, float duration, std::vector<float> curve)
            : m_type(type)
            , m_value(value)
            , m_time(time)
            , m_timeConstant(timeConstant)
            , m_duration(duration)
            , m_curve(curve)
        {
        }

        ParamEvent(const ParamEvent & rhs)
            : m_type(rhs.m_type)
            , m_value(rhs.m_value)
            , m_time(rhs.m_time)
            , m_timeConstant(rhs.m_timeConstant)
            , m_duration(rhs.m_duration)
            , m_curve(rhs.m_curve)
        {
        }

        const ParamEvent & operator=(const ParamEvent & rhs)
        {

            m_type = rhs.m_type;
            m_value = rhs.m_value;
            m_time = rhs.m_time;
            m_timeConstant = rhs.m_timeConstant;
            m_duration = rhs.m_duration;
            m_curve = rhs.m_curve;
            return *this;
        }

        unsigned type() const { return m_type; }
        float value() const { return m_value; }
        float time() const { return m_time; }
        float timeConstant() const { return m_timeConstant; }
        float duration() const { return m_duration; }
        std::vector<float> & curve() { return m_curve; }

    private:
        unsigned m_type;
        float m_value;
        float m_time;
        float m_timeConstant;
        float m_duration;
        std::vector<float> m_curve;
    };

    void insertEvent(const ParamEvent &);
    float valuesForTimeRangeImpl(double startTime, double endTime, float defaultValue,
                                 float * values, size_t numberOfValues, double sampleRate, double controlRate);

    std::vector<ParamEvent> m_events;
};

}  // namespace lab

#endif  // AudioParamTimeline_h
