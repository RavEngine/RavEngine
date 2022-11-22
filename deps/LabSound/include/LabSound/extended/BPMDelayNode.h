// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#pragma once

#ifndef BPM_DELAY_NODE_H
#define BPM_DELAY_NODE_H

#include "LabSound/core/AudioBasicProcessorNode.h"
#include "LabSound/core/AudioNode.h"
#include "LabSound/core/AudioSetting.h"
#include "LabSound/core/DelayNode.h"

namespace lab
{
class BPMDelay : public DelayNode
{
    float tempo;
    int noteDivision;
    std::vector<float> times;

    void recomputeDelay()
    {
        float dT = float(60.0f * noteDivision) / tempo;
        delayTime()->setFloat(dT);
    }

public:
    BPMDelay(AudioContext & ac, float tempo);
    virtual ~BPMDelay();

    void SetTempo(float newTempo)
    {
        tempo = newTempo;
        recomputeDelay();
    }

    void SetDelayIndex(TempoSync value);
};
}

#endif
