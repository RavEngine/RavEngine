// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef OscillatorNode_h
#define OscillatorNode_h

#include "LabSound/core/AudioParam.h"
#include "LabSound/core/AudioScheduledSourceNode.h"
#include "LabSound/core/Macros.h"
#include "LabSound/core/WaveTable.h"

namespace lab
{

class AudioBus;
class AudioContext;
class AudioSetting;

// params: frequency, detune, amplitude, and bias
// settings: type
//

// @TODO add duty param for the square wave to complete the oscillator, default value is 0.5

class OscillatorNode : public AudioScheduledSourceNode
{
    double phase = 0.0;
    virtual double tailTime(ContextRenderLock & r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock & r) const override { return 0; }
    virtual bool propagatesSilence(ContextRenderLock & r) const override;
    std::shared_ptr<AudioSetting> m_type;

public:
    OscillatorNode(AudioContext& ac);
    virtual ~OscillatorNode();

    static const char* static_name() { return "Oscillator"; }
    virtual const char* name() const override { return static_name(); }

    virtual void process(ContextRenderLock &, int bufferSize) override;
    virtual void reset(ContextRenderLock &) override { }

    OscillatorType type() const;
    void setType(OscillatorType type);

    std::shared_ptr<AudioParam> amplitude() { return m_amplitude; }
    std::shared_ptr<AudioParam> frequency() { return m_frequency; }
    std::shared_ptr<AudioParam> detune() { return m_detune; }
    std::shared_ptr<AudioParam> bias() { return m_bias; }

    std::shared_ptr<AudioParam> m_amplitude; // default 1.0
    std::shared_ptr<AudioParam> m_frequency; // hz
    std::shared_ptr<AudioParam> m_bias;      // default 0.0
    std::shared_ptr<AudioParam> m_detune;    // Detune value in Cents.

    void process_oscillator(ContextRenderLock & r, int bufferSize, int offset, int count);

    AudioFloatArray m_phaseIncrements;
    AudioFloatArray m_biasValues;
    AudioFloatArray m_detuneValues;
    AudioFloatArray m_amplitudeValues;
};

}  // namespace lab

#endif  // OscillatorNode_h
