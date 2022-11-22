// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#pragma once

#ifndef SFXR_NODE_H
#define SFXR_NODE_H

#include "LabSound/core/AudioScheduledSourceNode.h"
#include "LabSound/core/Macros.h"

namespace lab
{

class SfxrNode : public AudioScheduledSourceNode
{
public:
    SfxrNode(AudioContext & ac);
    virtual ~SfxrNode();

    static const char* static_name() { return "SFXR"; }
    virtual const char* name() const override { return static_name(); }

    // AudioNode
    virtual void process(ContextRenderLock &, int bufferSize) override;
    virtual void reset(ContextRenderLock &) override;

    std::shared_ptr<AudioSetting> preset() const { return _preset; }

    // SfxrNode - values in sfxr units
    std::shared_ptr<AudioSetting> waveType() const { return _waveType; }

    std::shared_ptr<AudioParam> attackTime() const { return _attack; }
    std::shared_ptr<AudioParam> sustainTime() const { return _sustainTime; }
    std::shared_ptr<AudioParam> sustainPunch() const { return _sustainPunch; }
    std::shared_ptr<AudioParam> decayTime() const { return _decayTime; }

    std::shared_ptr<AudioParam> startFrequency() const { return _startFrequency; }
    std::shared_ptr<AudioParam> minFrequency() const { return _minFrequency; }
    std::shared_ptr<AudioParam> slide() const { return _slide; }
    std::shared_ptr<AudioParam> deltaSlide() const { return _deltaSlide; }

    std::shared_ptr<AudioParam> vibratoDepth() const { return _vibratoDepth; }
    std::shared_ptr<AudioParam> vibratoSpeed() const { return _vibratoSpeed; }

    std::shared_ptr<AudioParam> changeAmount() const { return _changeAmount; }
    std::shared_ptr<AudioParam> changeSpeed() const { return _changeSpeed; }

    std::shared_ptr<AudioParam> squareDuty() const { return _squareDuty; }
    std::shared_ptr<AudioParam> dutySweep() const { return _dutySweep; }

    std::shared_ptr<AudioParam> repeatSpeed() const { return _repeatSpeed; }
    std::shared_ptr<AudioParam> phaserOffset() const { return _phaserOffset; }
    std::shared_ptr<AudioParam> phaserSweep() const { return _phaserSweep; }

    std::shared_ptr<AudioParam> lpFilterCutoff() const { return _lpFilterCutoff; }
    std::shared_ptr<AudioParam> lpFilterCutoffSweep() const { return _lpFilterCutoffSweep; }
    std::shared_ptr<AudioParam> lpFiterResonance() const { return _lpFiterResonance; }
    std::shared_ptr<AudioParam> hpFilterCutoff() const { return _hpFilterCutoff; }
    std::shared_ptr<AudioParam> hpFilterCutoffSweep() const { return _hpFilterCutoffSweep; }

    // utility functions that set the params with Hz values
    void setStartFrequencyInHz(float);
    void setVibratoSpeedInHz(float);

    // sfxr uses a lot of weird parameters. These are utility functions to help with that.
    float envelopeTimeInSeconds(float sfxrEnvTime);
    float envelopeTimeInSfxrUnits(float t);
    float frequencyInSfxrUnits(float hz);
    float frequencyInHz(float sfxr);
    float vibratoInSfxrUnits(float hz);
    float vibratoInHz(float sfxr);
    float filterFreqInHz(float sfxr);
    float filterFreqInSfxrUnits(float hz);

    enum WaveType
    {
        SQUARE = 0,
        SAWTOOTH,
        SINE,
        NOISE
    };

    // some presets
    void setDefaultBeep();
    void coin();
    void laser();
    void explosion();
    void powerUp();
    void hit();
    void jump();
    void select();

    // mutate the current sound
    void mutate();
    void randomize();

private:
    virtual bool propagatesSilence(ContextRenderLock & r) const override;
    virtual double tailTime(ContextRenderLock & r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock & r) const override { return 0; }

    std::shared_ptr<AudioSetting> _preset;
    std::shared_ptr<AudioSetting> _waveType;
    std::shared_ptr<AudioParam> _attack;
    std::shared_ptr<AudioParam> _sustainTime;
    std::shared_ptr<AudioParam> _sustainPunch;
    std::shared_ptr<AudioParam> _decayTime;
    std::shared_ptr<AudioParam> _startFrequency;
    std::shared_ptr<AudioParam> _minFrequency;
    std::shared_ptr<AudioParam> _slide;
    std::shared_ptr<AudioParam> _deltaSlide;
    std::shared_ptr<AudioParam> _vibratoDepth;
    std::shared_ptr<AudioParam> _vibratoSpeed;
    std::shared_ptr<AudioParam> _changeAmount;
    std::shared_ptr<AudioParam> _changeSpeed;
    std::shared_ptr<AudioParam> _squareDuty;
    std::shared_ptr<AudioParam> _dutySweep;
    std::shared_ptr<AudioParam> _repeatSpeed;
    std::shared_ptr<AudioParam> _phaserOffset;
    std::shared_ptr<AudioParam> _phaserSweep;
    std::shared_ptr<AudioParam> _lpFilterCutoff;
    std::shared_ptr<AudioParam> _lpFilterCutoffSweep;
    std::shared_ptr<AudioParam> _lpFiterResonance;
    std::shared_ptr<AudioParam> _hpFilterCutoff;
    std::shared_ptr<AudioParam> _hpFilterCutoffSweep;

    class Sfxr;
    Sfxr * sfxr;
};
}

#endif
