
// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015, The LabSound Authors. All rights reserved.

#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/AudioSetting.h"
#include "LabSound/core/GainNode.h"
#include "LabSound/core/OscillatorNode.h"

#include "LabSound/extended/AudioContextLock.h"
#include "LabSound/extended/SupersawNode.h"
#include "LabSound/extended/Registry.h"

#include <cfloat>

using namespace lab;

namespace lab
{

//////////////////////////////////////////
// Private Supersaw Node Implementation //
//////////////////////////////////////////

class SupersawNode::SupersawNodeInternal
{
public:
    SupersawNodeInternal(AudioContext & ac)
        : cachedDetune(FLT_MAX)
        , cachedFrequency(FLT_MAX)
    {
        gainNode = std::make_shared<GainNode>(ac);
        sawCount = std::make_shared<AudioSetting>("sawCount", "SAWC", AudioSetting::Type::Integer);
        sawCount->setUint32(1);
        detune = std::make_shared<AudioParam>("detune", "DTUN", 1.0, 0, 120);
        frequency = std::make_shared<AudioParam>("frequency", "FREQ", 440.0, 1.0f, sampleRate * 0.5f);
    }

    ~SupersawNodeInternal() = default;

    void update(ContextRenderLock & r)
    {
        /// @fixme these values should be per sample, not per quantum
        /// -or- they should be settings if they don't vary per sample
        if (cachedFrequency != frequency->value())
        {
            cachedFrequency = frequency->value();
            for (auto i : saws)
            {
                i->frequency()->setValue(cachedFrequency);
                i->frequency()->resetSmoothedValue();
            }
        }

        /// @fixme these values should be per sample, not per quantum
        /// -or- they should be settings if they don't vary per sample
        if (cachedDetune != detune->value())
        {
            cachedDetune = detune->value();
            float n = cachedDetune / ((float) saws.size() - 1.0f);
            for (size_t i = 0; i < saws.size(); ++i)
            {
                saws[i]->detune()->setValue(-cachedDetune + float(i) * 2 * n);
                saws[i]->detune()->resetSmoothedValue();
            }
        }
    }

    void update(ContextRenderLock & r, bool okayToReallocate)
    {
        size_t currentN = saws.size();
        int n = int(sawCount->valueUint32() + 0.5f);

        auto context = r.context();

        if (okayToReallocate && (n != currentN))
        {
            sampleRate = r.context()->sampleRate();

            // This implementation is similar to the technique illustrated here
            // https://noisehack.com/how-to-build-supersaw-synth-web-audio-api/
            //
            for (auto i : sawStorage)
            {
                context->disconnect(i, nullptr);
            }

            sawStorage.clear();
            saws.clear();

            for (int i = 0; i < n; ++i)
                sawStorage.emplace_back(std::make_shared<OscillatorNode>(*context));

            for (int i = 0; i < n; ++i)
                saws.push_back(sawStorage[i].get());

            for (auto i : sawStorage)
            {
                i->setType(OscillatorType::SAWTOOTH);
                context->connect(gainNode, i, 0, 0);
                i->start(0);
            }

            gainNode->gain()->setValue(1.f / float(n));

            cachedFrequency = FLT_MAX;
            cachedDetune = FLT_MAX;
        }

        update(r);
    }

    std::shared_ptr<GainNode> gainNode;
    std::shared_ptr<AudioParam> detune;
    std::shared_ptr<AudioParam> frequency;
    std::shared_ptr<AudioSetting> sawCount;

private:
    float sampleRate;
    float cachedDetune;
    float cachedFrequency;

    std::vector<std::shared_ptr<OscillatorNode>> sawStorage;
    std::vector<OscillatorNode *> saws;
};

//////////////////////////
// Public Supersaw Node //
//////////////////////////

SupersawNode::SupersawNode(AudioContext & ac)
    : AudioScheduledSourceNode(ac)
{
    internalNode.reset(new SupersawNodeInternal(ac));

    m_params.push_back(internalNode->detune);
    m_params.push_back(internalNode->frequency);
    m_settings.push_back(internalNode->sawCount);

    addOutput(std::unique_ptr<AudioNodeOutput>(new AudioNodeOutput(this, 1)));
    initialize();
}

SupersawNode::~SupersawNode()
{
    uninitialize();
}

void SupersawNode::process(ContextRenderLock & r, int bufferSize)
{
    internalNode->update(r, true);

    AudioBus * outputBus = output(0)->bus(r);

    if (!isInitialized() || !outputBus->numberOfChannels())
    {
        outputBus->zero();
        return;
    }

    AudioBus * dst = nullptr;
    internalNode->gainNode->input(0)->bus(r)->zero();
    /*AudioBus * renderedBus =*/ internalNode->gainNode->input(0)->pull(r, dst, bufferSize);
    internalNode->gainNode->process(r, bufferSize);
    AudioBus * inputBus = internalNode->gainNode->output(0)->bus(r);
    outputBus->copyFrom(*inputBus);
    outputBus->clearSilentFlag();
}

void SupersawNode::update(ContextRenderLock & r)
{
    internalNode->update(r, true);
}

std::shared_ptr<AudioParam> SupersawNode::detune() const
{
    return internalNode->detune;
}
std::shared_ptr<AudioParam> SupersawNode::frequency() const
{
    return internalNode->frequency;
}
std::shared_ptr<AudioSetting> SupersawNode::sawCount() const
{
    return internalNode->sawCount;
}

bool SupersawNode::propagatesSilence(ContextRenderLock & r) const
{
    return !isPlayingOrScheduled() || hasFinished();
}

}  // End namespace lab
