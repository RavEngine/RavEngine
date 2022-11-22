// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015, The LabSound Authors. All rights reserved.

#pragma once

#ifndef SPECTRAL_MONITOR_NODE_H
#define SPECTRAL_MONITOR_NODE_H

#include "LabSound/core/AudioBasicInspectorNode.h"
#include "LabSound/core/AudioContext.h"

#include <mutex>
#include <vector>

namespace lab
{
// params:
// settings: windowSize
class SpectralMonitorNode : public AudioBasicInspectorNode
{
    class SpectralMonitorNodeInternal;
    SpectralMonitorNodeInternal * internalNode = nullptr;

public:
    SpectralMonitorNode(AudioContext & ac);
    virtual ~SpectralMonitorNode();

    static const char* static_name() { return "SpectralMonitor"; }
    virtual const char* name() const override { return static_name(); }

    virtual void process(ContextRenderLock &, int bufferSize) override;
    virtual void reset(ContextRenderLock &) override;

    void spectralMag(std::vector<float> & result);
    void windowSize(unsigned int ws);
    unsigned int windowSize() const;

private:
    virtual double tailTime(ContextRenderLock & r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock & r) const override { return 0; }
};
}

#endif
