// License: BSD 2 Clause
// Copyright (C) 2011, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef DynamicsCompressorNode_h
#define DynamicsCompressorNode_h

#include "LabSound/core/AudioNode.h"
#include "LabSound/core/AudioParam.h"

namespace lab
{

class DynamicsCompressor;

// Fun trick:
//
// source ---> gain(100) ---> dynamics_compressor ---> output
//
// sounds like an old radio
//

class DynamicsCompressorNode : public AudioNode
{

public:
    DynamicsCompressorNode(AudioContext& ac);
    virtual ~DynamicsCompressorNode();

    static const char* static_name() { return "DynamicsCompressor"; }
    virtual const char* name() const override { return static_name(); }

    // AudioNode
    virtual void process(ContextRenderLock &, int bufferSize) override;
    virtual void reset(ContextRenderLock &) override;
    virtual void initialize() override;
    virtual void uninitialize() override;

    // Static compression curve parameters.
    std::shared_ptr<AudioParam> threshold() { return m_threshold; }
    std::shared_ptr<AudioParam> knee() { return m_knee; }
    std::shared_ptr<AudioParam> ratio() { return m_ratio; }
    std::shared_ptr<AudioParam> attack() { return m_attack; }
    std::shared_ptr<AudioParam> release() { return m_release; }

    // Amount by which the compressor is currently compressing the signal in decibels.
    std::shared_ptr<AudioParam> reduction() { return m_reduction; }

private:
    virtual double tailTime(ContextRenderLock & r) const override;
    virtual double latencyTime(ContextRenderLock & r) const override;

    std::unique_ptr<DynamicsCompressor> m_dynamicsCompressor;
    std::shared_ptr<AudioParam> m_threshold;
    std::shared_ptr<AudioParam> m_knee;
    std::shared_ptr<AudioParam> m_ratio;
    std::shared_ptr<AudioParam> m_reduction;
    std::shared_ptr<AudioParam> m_attack;
    std::shared_ptr<AudioParam> m_release;
};

}  // namespace lab

#endif  // DynamicsCompressorNode_h
