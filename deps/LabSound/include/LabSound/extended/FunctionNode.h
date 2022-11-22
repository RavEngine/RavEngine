// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef FUNCTION_NODE_H
#define FUNCTION_NODE_H

#include "LabSound/core/AudioNode.h"
#include "LabSound/core/AudioParam.h"
#include "LabSound/core/AudioScheduledSourceNode.h"

namespace lab
{

class FunctionNode : public AudioScheduledSourceNode
{

public:
    FunctionNode(AudioContext & ac, int channels = 1);
    virtual ~FunctionNode();

    static const char* static_name() { return "Function"; }
    virtual const char* name() const override { return static_name(); }

    void setFunction(std::function<void(ContextRenderLock & r, FunctionNode * me, int channel, float * buffer, int bufferSize)> fn)
    {
        _function = fn;
    }

    virtual void process(ContextRenderLock & r, int bufferSize) override;
    virtual void reset(ContextRenderLock & r) override;

    double now() const { return _now; }

private:
    virtual bool propagatesSilence(ContextRenderLock & r) const override;
    virtual double tailTime(ContextRenderLock & r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock & r) const override { return 0; }

    std::function<void(ContextRenderLock & r, FunctionNode * me, int channel, float * values, int bufferSize)> _function;

    double _now = 0.0;
};

}  // end namespace lab

#endif
