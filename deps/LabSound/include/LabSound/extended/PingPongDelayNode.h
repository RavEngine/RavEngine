// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#pragma once

#ifndef PING_PONG_DELAY_NODE_H
#define PING_PONG_DELAY_NODE_H

#include "LabSound/core/AudioNode.h"
#include "LabSound/core/AudioParam.h"
#include "LabSound/core/ChannelMergerNode.h"
#include "LabSound/core/ChannelSplitterNode.h"
#include "LabSound/core/GainNode.h"
#include "LabSound/extended/BPMDelayNode.h"

namespace lab
{
class ContextGraphLock;
class AudioContext;

class Subgraph
{
public:
    std::shared_ptr<GainNode> output;
    std::shared_ptr<GainNode> input;
    virtual void BuildSubgraph(AudioContext & ac) = 0;
    virtual ~Subgraph() {}
};

class PingPongDelayNode : public Subgraph
{
    float tempo;

    std::shared_ptr<BPMDelay> leftDelay;
    std::shared_ptr<BPMDelay> rightDelay;

    std::shared_ptr<GainNode> splitterGain;
    std::shared_ptr<GainNode> wetGain;
    std::shared_ptr<GainNode> feedbackGain;

    std::shared_ptr<ChannelMergerNode> merger;
    std::shared_ptr<ChannelSplitterNode> splitter;

public:
    PingPongDelayNode(AudioContext &, float tempo);

    void SetTempo(float t);
    void SetFeedback(float f);
    void SetLevel(float f);
    void SetDelayIndex(TempoSync value);

    virtual void BuildSubgraph(AudioContext & ac) override;
};
}

#endif
