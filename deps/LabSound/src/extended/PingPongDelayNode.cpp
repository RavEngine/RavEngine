// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/AudioProcessor.h"
#include "LabSound/core/Macros.h"

#include "LabSound/extended/AudioContextLock.h"
#include "LabSound/extended/PingPongDelayNode.h"
#include "LabSound/extended/Util.h"

#include <algorithm>

using namespace lab;

namespace lab
{
PingPongDelayNode::PingPongDelayNode(AudioContext & ac, float tempo)
{
    input = std::make_shared<lab::GainNode>(ac);
    output = std::make_shared<lab::GainNode>(ac);

    leftDelay = std::make_shared<lab::BPMDelay>(ac, tempo);
    rightDelay = std::make_shared<lab::BPMDelay>(ac, tempo);

    splitterGain = std::make_shared<lab::GainNode>(ac);
    wetGain = std::make_shared<lab::GainNode>(ac);
    feedbackGain = std::make_shared<lab::GainNode>(ac);

    merger = std::make_shared<lab::ChannelMergerNode>(ac, 2);
    splitter = std::make_shared<lab::ChannelSplitterNode>(ac, 2);

    SetDelayIndex(TempoSync::TS_8);
    SetFeedback(0.5f);
    SetLevel(1.0f);
}

void PingPongDelayNode::SetTempo(float t)
{
    tempo = t;
    leftDelay->SetTempo(tempo);
    rightDelay->SetTempo(tempo);
}

void PingPongDelayNode::SetFeedback(float f)
{
    auto clamped = clampTo<float>(f, 0.0f, 1.0f);
    feedbackGain->gain()->setValue(clamped);
}

void PingPongDelayNode::SetLevel(float f)
{
    auto clamped = clampTo<float>(f, 0.0f, 1.0f);
    wetGain->gain()->setValue(clamped);
}

void PingPongDelayNode::SetDelayIndex(TempoSync value)
{
    leftDelay->SetDelayIndex(value);
    rightDelay->SetDelayIndex(value);
}

void PingPongDelayNode::BuildSubgraph(AudioContext & ac)
{
    // Input into splitter
    ac.connect(splitter, input, 0, 0);

    ac.connect(splitterGain, splitter, 0, 0);
    ac.connect(splitterGain, splitter, 1, 0);

    ac.connect(wetGain, splitterGain, 0, 0);
    splitterGain->gain()->setValue(0.5f);

    ac.connect(leftDelay, wetGain, 0, 0);
    ac.connect(leftDelay, feedbackGain, 0, 0);

    ac.connect(rightDelay, leftDelay, 0, 0);
    ac.connect(feedbackGain, rightDelay, 0, 0);

    ac.connect(merger, leftDelay, 0, 0);
    ac.connect(merger, rightDelay, 0, 1);

    ac.connect(output, merger, 0, 0);

    // Activate with input->output
    ac.connect(output, input, 0, 0);
}
}
