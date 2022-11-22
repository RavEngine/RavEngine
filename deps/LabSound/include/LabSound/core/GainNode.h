// License: BSD 2 Clause
// Copyright (C) 2011, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef GainNode_h
#define GainNode_h

#include "LabSound/core/AudioArray.h"
#include "LabSound/core/AudioNode.h"
#include "LabSound/core/AudioParam.h"

namespace lab
{

class AudioContext;

// GainNode is an AudioNode with one input and one output which applies a gain (volume) change to the audio signal.
// De-zippering (smoothing) is applied when the gain value is changed dynamically.
//
// params: gain
// settings:
//
class GainNode : public AudioNode
{
public:
    GainNode(AudioContext& ac);
    virtual ~GainNode();

    static const char* static_name() { return "GainNode"; }
    virtual const char* name() const override { return static_name(); }

    // AudioNode
    virtual void process(ContextRenderLock &, int bufferSize) override;
    virtual void reset(ContextRenderLock &) override;

    std::shared_ptr<AudioParam> gain() const { return m_gain; }

protected:
    virtual double tailTime(ContextRenderLock & r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock & r) const override { return 0; }

    float m_lastGain;  // for de-zippering
    std::shared_ptr<AudioParam> m_gain;

    AudioFloatArray m_sampleAccurateGainValues;
};

}  // namespace lab

#endif  // GainNode_h
