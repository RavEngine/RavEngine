// License: The MIT License
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.
// Copyright (C) 2013, Two Big Ears Ltd (http://twobigears.com)

#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/AudioProcessor.h"

#include "LabSound/extended/AudioContextLock.h"
#include "LabSound/extended/PeakCompNode.h"

#include "internal/VectorMath.h"

#include "LabSound/core/Macros.h"

#include <algorithm>
#include <vector>

using namespace lab;

namespace lab
{

/////////////////////////////////////////
// Prviate PeakCompNode Implementation //
/////////////////////////////////////////

class PeakCompNode::PeakCompNodeInternal : public AudioProcessor
{

public:
    PeakCompNodeInternal() : AudioProcessor()
    {
        m_threshold = std::make_shared<AudioParam>("threshold", "THRS", 0, 0, -1e6f);
        m_ratio = std::make_shared<AudioParam>("ratio", "RATI", 1, 0, 10);
        m_attack = std::make_shared<AudioParam>("attack", "ATCK", 0.001f, 0, 1000);
        m_release = std::make_shared<AudioParam>("release", "RELS", 0.001f, 0, 1000);
        m_makeup = std::make_shared<AudioParam>("makeup", "MAKE", 0, 0, 60);
        m_knee = std::make_shared<AudioParam>("knee", "KNEE", 0, 0, 1);

        for (int i = 0; i < 2; i++)
        {
            kneeRecursive[i] = 0.f;
            attackRecursive[i] = 0.f;
            releaseRecursive[i] = 0.f;
        }
    }

    virtual ~PeakCompNodeInternal() {}

    virtual void initialize() override {}

    virtual void uninitialize() override {}

    // Processes the source to destination bus.  The number of channels must match in source and destination.
    virtual void process(ContextRenderLock & r, const lab::AudioBus * sourceBus, lab::AudioBus * destinationBus, int framesToProcess) override
    {
        int sourceNumChannels = sourceBus->numberOfChannels();
        int destNumChannels = destinationBus->numberOfChannels();

        if (!destNumChannels)
            return;

        // Get sample rate
        internalSampleRate = r.context()->sampleRate();
        oneOverSampleRate = 1.0 / internalSampleRate;

        // copy attributes to run time variables
        /// @fixme these values should be per sample, not per quantum
        /// -or- they should be settings if they don't vary per sample
        float v = m_threshold->value();
        if (v <= 0)
        {
            // dB to linear (could use the function from m_pd.h)
            threshold = powf(10.f, (v * 0.05f));
        }
        else
            threshold = 0;

        /// @fixme these values should be per sample, not per quantum
        /// -or- they should be settings if they don't vary per sample
        v = m_ratio->value();
        if (v >= 1)
        {
            ratio = 1.f / v;
        }
        else
            ratio = 1;

        /// @fixme these values should be per sample, not per quantum
        /// -or- they should be settings if they don't vary per sample
        v = m_attack->value();
        if (v >= 0.001f)
        {
            attack = v * 0.001;
        }
        else
            attack = 0.000001;

        /// @fixme these values should be per sample, not per quantum
        /// -or- they should be settings if they don't vary per sample
        v = m_release->value();
        if (v >= 0.001f)
        {
            release = v * 0.001;
        }
        else
            release = 0.000001;

        /// @fixme these values should be per sample, not per quantum
        /// -or- they should be settings if they don't vary per sample
        v = m_makeup->value();
        // dB to linear (could use the function from m_pd.h)
        makeupGain = powf(10.f, (v * 0.05f));

        /// @fixme these values should be per sample, not per quantum
        /// -or- they should be settings if they don't vary per sample
        v = m_knee->value();
        if (v >= 0 && v <= 1)
        {
            // knee value (0 to 1) is scaled from 0 (hard) to 0.02 (smooth). Could be scaled to a larger number.
            knee = v * 0.02;
        }

        // calc coefficients from run time vars
        kneeCoeffs = exp(0. - (oneOverSampleRate / knee));
        kneeCoeffsMinus = 1. - kneeCoeffs;

        attackCoeffs = exp(0. - (oneOverSampleRate / attack));
        attackCoeffsMinus = 1. - attackCoeffs;

        releaseCoeff = exp(0. - (oneOverSampleRate / release));
        releaseCoeffMinus = 1. - releaseCoeff;

        // Handle both the 1 -> N and N -> N case here.
        const float * source[16];
        for (int i = 0; i < sourceNumChannels; ++i)
        {
            if (sourceBus->numberOfChannels() == sourceNumChannels)
                source[i] = sourceBus->channel(i)->data();
            else
                source[i] = sourceBus->channel(0)->data();
        }

        float * dest[16];
        for (int i = 0; i < destNumChannels; ++i)
            dest[i] = destinationBus->channel(i)->mutableData();

        for (int i = 0; i < framesToProcess; ++i)
        {
            float peakEnv = 0;
            for (int j = 0; j < sourceNumChannels; ++j)
            {
                peakEnv += source[j][i];
            }
            // Release recursive
            releaseRecursive[0] = (releaseCoeffMinus * peakEnv) + (releaseCoeff * std::max(peakEnv, float(releaseRecursive[1])));

            // Attack recursive
            attackRecursive[0] = ((attackCoeffsMinus * releaseRecursive[0]) + (attackCoeffs * attackRecursive[1])) + 0.000001f;  // avoid div by 0

            // Knee smoothening and gain reduction
            kneeRecursive[0] = (kneeCoeffsMinus * std::max(std::min(((threshold + (ratio * (attackRecursive[0] - threshold))) / attackRecursive[0]), 1.), 0.)) + (kneeCoeffs * kneeRecursive[1]);

            int k = std::min(sourceNumChannels, destNumChannels);
            for (int j = 0; j < k; ++j)
            {
                dest[j][i] = static_cast<float>(source[j][i] * kneeRecursive[0] * makeupGain);
            }
        }

        releaseRecursive[1] = releaseRecursive[0];
        attackRecursive[1] = attackRecursive[0];
        kneeRecursive[1] = kneeRecursive[0];
    }

    float internalSampleRate;
    double oneOverSampleRate;

    // Arrays for delay lines
    double kneeRecursive[2];
    double attackRecursive[2];
    double releaseRecursive[2];

    double attack;
    double release;
    double ratio;
    double threshold;

    double knee;
    double kneeCoeffs;
    double kneeCoeffsMinus;

    double attackCoeffs;
    double attackCoeffsMinus;

    double releaseCoeff;
    double releaseCoeffMinus;

    double makeupGain;

    // Resets filter state
    virtual void reset() override
    { 
        // @tofix
    }

    virtual double tailTime(ContextRenderLock & r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock & r) const override { return 0; }

    std::shared_ptr<AudioParam> m_threshold;
    std::shared_ptr<AudioParam> m_ratio;
    std::shared_ptr<AudioParam> m_attack;
    std::shared_ptr<AudioParam> m_release;
    std::shared_ptr<AudioParam> m_makeup;
    std::shared_ptr<AudioParam> m_knee;
};

std::shared_ptr<AudioParam> PeakCompNode::threshold() const
{
    return internalNode->m_threshold;
}

std::shared_ptr<AudioParam> PeakCompNode::ratio() const
{
    return internalNode->m_ratio;
}

std::shared_ptr<AudioParam> PeakCompNode::attack() const
{
    return internalNode->m_attack;
}

std::shared_ptr<AudioParam> PeakCompNode::release() const
{
    return internalNode->m_release;
}

std::shared_ptr<AudioParam> PeakCompNode::makeup() const
{
    return internalNode->m_makeup;
}

std::shared_ptr<AudioParam> PeakCompNode::knee() const
{
    return internalNode->m_knee;
}

/////////////////////////
// Public PeakCompNode //
/////////////////////////

PeakCompNode::PeakCompNode(AudioContext & ac)
    : lab::AudioBasicProcessorNode(ac)
{
    m_processor.reset(new PeakCompNodeInternal());

    internalNode = static_cast<PeakCompNodeInternal *>(m_processor.get());

    m_params.push_back(internalNode->m_threshold);
    m_params.push_back(internalNode->m_ratio);
    m_params.push_back(internalNode->m_attack);
    m_params.push_back(internalNode->m_release);
    m_params.push_back(internalNode->m_makeup);
    m_params.push_back(internalNode->m_knee);

    initialize();
}

PeakCompNode::~PeakCompNode()
{
    uninitialize();
}

}  // End namespace lab
