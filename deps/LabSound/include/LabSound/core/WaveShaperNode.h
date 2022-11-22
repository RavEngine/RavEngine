
// License: BSD 2 Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef WaveShaperNode_h
#define WaveShaperNode_h

#include "LabSound/core/AudioNode.h"
#include <vector>

namespace lab {

class WaveShaperNode : public AudioNode
{
public:
    WaveShaperNode(AudioContext & ac);
    virtual ~WaveShaperNode();

    static const char* static_name() { return "WaveShaper"; }
    virtual const char* name() const override { return static_name(); }

    // copies the curve
    void setCurve(std::vector<float> & curve);

    // AudioNode
    virtual void process(ContextRenderLock &, int bufferSize) override;
    virtual void reset(ContextRenderLock&) override {}

protected:
    void processBuffer(ContextRenderLock&, const float* source, float* destination, int framesToProcess);
    virtual double tailTime(ContextRenderLock& r) const override { return 0.; }
    virtual double latencyTime(ContextRenderLock& r) const override { return 0.; }

    std::vector<float> m_curve;
    std::vector<float>* m_newCurve = nullptr;
};

}  // namespace lab

#endif
