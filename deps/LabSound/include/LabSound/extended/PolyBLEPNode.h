// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2020+, The LabSound Authors. All rights reserved.

#ifndef lab_poly_blep_node_h
#define lab_poly_blep_node_h

#include "LabSound/core/AudioParam.h"
#include "LabSound/core/AudioScheduledSourceNode.h"
#include "LabSound/core/Macros.h"
#include "LabSound/core/WaveTable.h"

namespace lab
{

class AudioBus;
class AudioContext;
class AudioSetting;

/*
 * Anti-aliased (sometimes called virtual analog) digital oscillators that approximate the behavior of 
 * their analog counterparters is an active area of DSP research. Some seminal techniques are outlined
 * below with several references that inform this implementation. 
 *     \ref Band-limited Impulse Trains (BLIT) (Stilson & Smith 1996)
 *     \ref Band-limited Step Functions (BLEP) (Brandt 2001, Leary & Bright 2009)
 *     \ref MiniBLEP (minimum phase lowpass filtered prior to BLEP integration) (Brandt 2001)
 *     \ref Differentiated Parabolic Waveform Oscillators (DPW) (Valimaki 2010)
 *     \ref Polynomial Band-limited Step Functions (PolyBLEP) (Valimaki et. al 2010)
 *     \ref Improved Polynomial Transition Regions Algorithm for Alias-Suppressed Signal Synthesis(EPTR) (Ambrits and Bank, 2013)
 *     \ref Rounding Corners with BLAMP (PolyBLAMP) - (Esqueda, Valimaki, et. al 2016)
 * [1] http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
 * [2] http://metafunction.co.uk/all-about-digital-oscillators-part-2-blits-bleps/
 * [3] https://www.experimentalscene.com/articles/minbleps.php
 */

enum class PolyBLEPType
{
    TRIANGLE,
    SQUARE,
    RECTANGLE,
    SAWTOOTH,
    RAMP,
    MODIFIED_TRIANGLE,
    MODIFIED_SQUARE,
    HALF_WAVE_RECTIFIED_SINE,
    FULL_WAVE_RECTIFIED_SINE,
    TRIANGULAR_PULSE,
    TRAPEZOID_FIXED,
    TRAPEZOID_VARIABLE,
    _PolyBLEPCount
};

class PolyBlepImpl;

class PolyBLEPNode : public AudioScheduledSourceNode
{
    virtual double tailTime(ContextRenderLock & r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock & r) const override { return 0; }
    virtual bool propagatesSilence(ContextRenderLock & r) const override;
    std::shared_ptr<AudioSetting> m_type;
    std::unique_ptr<PolyBlepImpl> polyblep;

public:
    PolyBLEPNode(AudioContext & ac);
    virtual ~PolyBLEPNode();

    static const char* static_name() { return "PolyBLEP"; }
    virtual const char* name() const override { return static_name(); }

    virtual void process(ContextRenderLock &, int bufferSize) override;
    virtual void reset(ContextRenderLock &) override { }

    PolyBLEPType type() const;
    void setType(PolyBLEPType type);

    std::shared_ptr<AudioParam> amplitude() { return m_amplitude; }
    std::shared_ptr<AudioParam> frequency() { return m_frequency; }

    std::shared_ptr<AudioParam> m_amplitude; // default 1.0
    std::shared_ptr<AudioParam> m_frequency; // hz

    void processPolyBLEP(ContextRenderLock & r, int bufferSize, int offset, int count);

    AudioFloatArray m_amplitudeValues;
};

}  // namespace lab

#endif  // lab_poly_blep_node_h
