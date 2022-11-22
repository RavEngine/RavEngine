// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef CLIP_NODE_H
#define CLIP_NODE_H

#include "LabSound/core/AudioBasicProcessorNode.h"
#include "LabSound/core/AudioParam.h"

namespace lab
{
// ClipNode clips a signal, using either thresholding or tanh
//
// params: a, b
// settings: mode
//
class ClipNode : public AudioBasicProcessorNode
{
    class ClipNodeInternal;
    ClipNodeInternal * internalNode;

public:
    enum Mode
    {
        CLIP = 0,
        TANH = 1,
        _Count = 2
    };

    ClipNode(AudioContext & ac);
    virtual ~ClipNode();

    static const char* static_name() { return "Clip"; }
    virtual const char* name() const override { return static_name(); }

    void setMode(Mode m);

    // in CLIP mode, a is the min value, and b is the max value.
    // in TANH mode, a is the overall gain, and b is the input gain.
    // The higher the input gain the more severe the distortion.
    std::shared_ptr<AudioParam> aVal();
    std::shared_ptr<AudioParam> bVal();
};
}

#endif
