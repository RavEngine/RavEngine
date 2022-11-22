// License: BSD 2 Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/WaveShaperNode.h"
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/extended/Registry.h"
#include "internal/Assertions.h"
#include <algorithm>
#include <memory>
#include <mutex>
#include <vector>

namespace lab {

WaveShaperNode::WaveShaperNode(AudioContext& ac)
: AudioNode(ac)
{
    addInput(std::unique_ptr<AudioNodeInput>(new AudioNodeInput(this)));
    addOutput(std::unique_ptr<AudioNodeOutput>(new AudioNodeOutput(this, 1)));
    initialize();
}

WaveShaperNode::~WaveShaperNode()
{
    if (m_newCurve)
    {
        // @TODO mutex
        delete m_newCurve;
    }
}

void WaveShaperNode::setCurve(std::vector<float> & curve)
{
    std::vector<float>* new_curve = new std::vector<float>();
    *new_curve = curve;
    if (m_newCurve)
    {
        // @TODO mutex
        std::vector<float>* x = nullptr;
        std::swap(x, m_newCurve);
        delete x;
    }
    m_newCurve = new_curve;
}

void WaveShaperNode::processBuffer(ContextRenderLock&, const float* source, float* destination, int framesToProcess)
{
    float const* curveData = m_curve.data();
    int curveLength = static_cast<int>(m_curve.size());

    ASSERT(curveData);

    if (!curveData || !curveLength)
    {
        memcpy(destination, source, sizeof(float) * framesToProcess);
        return;
    }

    // Apply waveshaping curve.
    for (int i = 0; i < framesToProcess; ++i)
    {
        const float input = source[i];

        // Calculate an index based on input -1 -> +1 with 0 being at the center of the curve data.
        int index = static_cast<size_t>((curveLength * (input + 1)) / 2);

        // Clip index to the input range of the curve.
        // This takes care of input outside of nominal range -1 -> +1
        index = std::max(index, 0);
        index = std::min(index, curveLength - 1);
        destination[i] = curveData[index];
    }
}

void WaveShaperNode::process(ContextRenderLock & r, int bufferSize)
{
    if (m_newCurve)
    {
        // @TODO mutex
        std::vector<float>* x = nullptr;
        std::swap(x, m_newCurve);
        m_curve = *x;
        delete x;
    }

    AudioBus* destinationBus = output(0)->bus(r);
    if (!isInitialized() || !m_curve.size())
    {
        destinationBus->zero();
        return;
    }

    AudioBus* sourceBus = input(0)->bus(r);
    if (!input(0)->isConnected())
    {
        destinationBus->zero();
        return;
    }

    int srcChannelCount = sourceBus->numberOfChannels();
    int dstChannelCount = destinationBus->numberOfChannels();
    if (srcChannelCount != dstChannelCount)
    {
        output(0)->setNumberOfChannels(r, srcChannelCount);
        destinationBus = output(0)->bus(r);
    }

    for (int i = 0; i < srcChannelCount; ++i)
    {
        processBuffer(r, sourceBus->channel(i)->data(), destinationBus->channel(i)->mutableData(), bufferSize);
    }
}

}  // namespace lab
