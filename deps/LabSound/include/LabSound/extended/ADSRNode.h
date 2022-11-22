// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015, The LabSound Authors. All rights reserved.

#ifndef ADSR_NODE_H
#define ADSR_NODE_H

#include "LabSound/core/AudioNode.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/core/AudioSetting.h"

namespace lab
{


class ADSRNode : public AudioNode
{
    class ADSRNodeImpl;
    ADSRNodeImpl * adsr_impl;

public:
    ADSRNode(AudioContext &);
    virtual ~ADSRNode();

    static const char* static_name() { return "ADSR"; }
    virtual const char* name() const override { return static_name(); }

    // This function will return true after the release period (only if a noteOff has been issued). 
    bool finished(ContextRenderLock &);

    void set(float attack_time, float attack_level, float decay_time, float sustain_time, float sustain_level, float release_time);

    virtual void process(ContextRenderLock& r, int bufferSize) override;
    virtual void reset(ContextRenderLock&) override;
    virtual double tailTime(ContextRenderLock& r) const override { return 0.; }
    virtual double latencyTime(ContextRenderLock& r) const override { return 0.f; }

    std::shared_ptr<AudioParam> gate() const; // gate signal

    std::shared_ptr<AudioSetting> oneShot() const;  // If zero, gate controls attack and sustain, else sustainTime controls sustain
    std::shared_ptr<AudioSetting> attackTime() const;  // Duration in seconds
    std::shared_ptr<AudioSetting> attackLevel() const;  // Level
    std::shared_ptr<AudioSetting> decayTime() const;  // Duration in seconds
    std::shared_ptr<AudioSetting> sustainTime() const;  // Duration in seconds
    std::shared_ptr<AudioSetting> sustainLevel() const;  // Level
    std::shared_ptr<AudioSetting> releaseTime() const;  // Duration in seconds
};

}

#endif
