
// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015, The LabSound Authors. All rights reserved.

#ifndef SUPERSAW_NODE_H
#define SUPERSAW_NODE_H

#include "LabSound/core/AudioContext.h"
#include "LabSound/core/AudioNode.h"
#include "LabSound/core/AudioParam.h"
#include "LabSound/core/AudioScheduledSourceNode.h"

namespace lab
{
class SupersawNode : public AudioScheduledSourceNode
{
    class SupersawNodeInternal;
    std::unique_ptr<SupersawNodeInternal> internalNode;

public:
    SupersawNode(AudioContext & ac);
    virtual ~SupersawNode();

    static const char* static_name() { return "SuperSaw"; }
    virtual const char* name() const override { return static_name(); }

    std::shared_ptr<AudioSetting> sawCount() const;
    std::shared_ptr<AudioParam> frequency() const;
    std::shared_ptr<AudioParam> detune() const;

    void update(ContextRenderLock & r);  // call if sawCount is changed. CBB: update automatically

private:
    virtual void process(ContextRenderLock &, int bufferSize) override;

    virtual void reset(ContextRenderLock &) override {}

    virtual double tailTime(ContextRenderLock & r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock & r) const override { return 0; }

    virtual bool propagatesSilence(ContextRenderLock & r) const override;
};
}

#endif
