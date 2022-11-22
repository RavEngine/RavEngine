// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#pragma once

#ifndef PWM_NODE_H
#define PWM_NODE_H

#include "LabSound/core/AudioBasicProcessorNode.h"
#include "LabSound/core/AudioParam.h"

namespace lab
{

// PWMNode implements a comparison based PWM. That could be improved.
// Expects two inputs.
// input 0 is the carrier, and input 1 is the modulator.
// If there is no modulator, then the node is a pass-through.
class PWMNode : public AudioBasicProcessorNode
{
    class PWMNodeInternal;
    PWMNodeInternal * internalNode;

public:
    PWMNode(AudioContext & ac);
    virtual ~PWMNode();
    static const char* static_name() { return "PWM"; }
    virtual const char* name() const override { return static_name(); }
};
}

#endif
