// License: BSD 3 Clause
// Copyright (C) 2011, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "internal/DynamicsCompressor.h"
#include "internal/Assertions.h"
#include "internal/AudioUtilities.h"

#include "LabSound/core/AudioBus.h"
#include "LabSound/extended/AudioContextLock.h"

#include "LabSound/core/Macros.h"

namespace lab
{

using namespace AudioUtilities;

DynamicsCompressor::DynamicsCompressor(unsigned numberOfChannels)
    : m_numberOfChannels(numberOfChannels)
    , m_compressor(numberOfChannels)
{
    // Uninitialized state - for parameter recalculation.
    m_lastFilterStageRatio = -1;
    m_lastAnchor = -1;
    m_lastFilterStageGain = -1;

    setNumberOfChannels(numberOfChannels);
    initializeParameters();
}

void DynamicsCompressor::setParameterValue(unsigned parameterID, float value)
{
    ASSERT(parameterID < ParamLast);
    if (parameterID < ParamLast)
        m_parameters[parameterID] = value;
}

void DynamicsCompressor::initializeParameters()
{
    // Initializes compressor to default values.

    m_parameters[ParamThreshold] = -24;  // dB
    m_parameters[ParamKnee] = 30;  // dB
    m_parameters[ParamRatio] = 12;  // unit-less
    m_parameters[ParamAttack] = 0.003f;  // seconds
    m_parameters[ParamRelease] = 0.250f;  // seconds
    m_parameters[ParamPreDelay] = 0.006f;  // seconds

    // Release zone values 0 -> 1.
    m_parameters[ParamReleaseZone1] = 0.09f;
    m_parameters[ParamReleaseZone2] = 0.16f;
    m_parameters[ParamReleaseZone3] = 0.42f;
    m_parameters[ParamReleaseZone4] = 0.98f;

    m_parameters[ParamFilterStageGain] = 4.4f;  // dB
    m_parameters[ParamFilterStageRatio] = 2;
    m_parameters[ParamFilterAnchor] = 15000.f / 22500.f;  // dimitri sample rate

    m_parameters[ParamPostGain] = 0;  // dB
    m_parameters[ParamReduction] = 0;  // dB

    // Linear crossfade (0 -> 1).
    m_parameters[ParamEffectBlend] = 1;
}

float DynamicsCompressor::parameterValue(unsigned parameterID)
{
    ASSERT(parameterID < ParamLast);
    return m_parameters[parameterID];
}

void DynamicsCompressor::setEmphasisStageParameters(unsigned stageIndex, float gain, float normalizedFrequency /* 0 -> 1 */)
{
    float gk = 1 - gain / 20;
    float f1 = normalizedFrequency * gk;
    float f2 = normalizedFrequency / gk;
    float r1 = expf(-f1 * static_cast<float>(LAB_PI));
    float r2 = expf(-f2 * static_cast<float>(LAB_PI));

    ASSERT(m_numberOfChannels == m_preFilterPacks.size());

    for (unsigned i = 0; i < m_numberOfChannels; ++i)
    {
        // Set pre-filter zero and pole to create an emphasis filter.
        ZeroPole & preFilter = m_preFilterPacks[i]->filters[stageIndex];
        preFilter.setZero(r1);
        preFilter.setPole(r2);

        // Set post-filter with zero and pole reversed to create the de-emphasis filter.
        // If there were no compressor kernel in between, they would cancel each other out (allpass filter).
        ZeroPole & postFilter = m_postFilterPacks[i]->filters[stageIndex];
        postFilter.setZero(r2);
        postFilter.setPole(r1);
    }
}

void DynamicsCompressor::setEmphasisParameters(float gain, float anchorFreq, float filterStageRatio)
{
    setEmphasisStageParameters(0, gain, anchorFreq);
    setEmphasisStageParameters(1, gain, anchorFreq / filterStageRatio);
    setEmphasisStageParameters(2, gain, anchorFreq / (filterStageRatio * filterStageRatio));
    setEmphasisStageParameters(3, gain, anchorFreq / (filterStageRatio * filterStageRatio * filterStageRatio));
}

void DynamicsCompressor::process(ContextRenderLock & r, const AudioBus * sourceBus, AudioBus * destinationBus, int bufferSize, int offset, int count)
{
    int numberOfDestChannels = destinationBus->numberOfChannels();
    int numberOfSourceChannels = sourceBus->numberOfChannels();

    if (!numberOfSourceChannels)
    {
        destinationBus->zero();
        return;
    }

    if (numberOfDestChannels != m_numberOfChannels)
    {
        setNumberOfChannels(numberOfSourceChannels);
        numberOfDestChannels = numberOfSourceChannels;
        for (int i = 0; i < numberOfDestChannels; ++i)
        {
            m_sourceChannels[i] = sourceBus->channel(i)->data();
            m_destinationChannels[i] = destinationBus->channel(i)->mutableData();
        }
    }

    for (int i = 0; i < numberOfDestChannels; ++i)
    {
        m_sourceChannels[i] = sourceBus->channel(i)->data();
        m_destinationChannels[i] = destinationBus->channel(i)->mutableData();
    }

    float filterStageGain = parameterValue(ParamFilterStageGain);
    float filterStageRatio = parameterValue(ParamFilterStageRatio);
    float anchor = parameterValue(ParamFilterAnchor);

    if (filterStageGain != m_lastFilterStageGain || filterStageRatio != m_lastFilterStageRatio || anchor != m_lastAnchor)
    {
        m_lastFilterStageGain = filterStageGain;
        m_lastFilterStageRatio = filterStageRatio;
        m_lastAnchor = anchor;

        setEmphasisParameters(filterStageGain, anchor, filterStageRatio);
    }

    // Apply pre-emphasis filter.
    // Note that the final three stages are computed in-place in the destination buffer.
    for (int i = 0; i < numberOfDestChannels; ++i)
    {
        const float * sourceData = m_sourceChannels[i];
        float * destinationData = m_destinationChannels[i];
        ZeroPole * preFilters = m_preFilterPacks[i]->filters;

        preFilters[0].process(sourceData, destinationData, bufferSize, offset, count);
        preFilters[1].process(destinationData, destinationData, bufferSize, offset, count);
        preFilters[2].process(destinationData, destinationData, bufferSize, offset, count);
        preFilters[3].process(destinationData, destinationData, bufferSize, offset, count);
    }

    float dbThreshold = parameterValue(ParamThreshold);
    float dbKnee = parameterValue(ParamKnee);
    float ratio = parameterValue(ParamRatio);
    float attackTime = parameterValue(ParamAttack);
    float releaseTime = parameterValue(ParamRelease);
    float preDelayTime = parameterValue(ParamPreDelay);

    // This is effectively a master volume on the compressed signal (pre-blending).
    float dbPostGain = parameterValue(ParamPostGain);

    // Linear blending value from dry to completely processed (0 -> 1)
    // 0 means the signal is completely unprocessed.
    // 1 mixes in only the compressed signal.
    float effectBlend = parameterValue(ParamEffectBlend);

    float releaseZone1 = parameterValue(ParamReleaseZone1);
    float releaseZone2 = parameterValue(ParamReleaseZone2);
    float releaseZone3 = parameterValue(ParamReleaseZone3);
    float releaseZone4 = parameterValue(ParamReleaseZone4);

    // Apply compression to the pre-filtered signal. The processing is performed in place.
    m_compressor.process(r,
                         m_sourceChannels.get(),
                         m_destinationChannels.get(),
                         numberOfDestChannels,
                         bufferSize,
                         dbThreshold,
                         dbKnee,
                         ratio,
                         attackTime,
                         releaseTime,
                         preDelayTime,
                         dbPostGain,
                         effectBlend,
                         releaseZone1,
                         releaseZone2,
                         releaseZone3,
                         releaseZone4);

    // Update the compression amount.
    setParameterValue(ParamReduction, m_compressor.meteringGain());

    // Apply de-emphasis filter.
    for (int i = 0; i < numberOfDestChannels; ++i)
    {
        float * destinationData = m_destinationChannels[i];
        ZeroPole * postFilters = m_postFilterPacks[i]->filters;

        postFilters[0].process(destinationData, destinationData, bufferSize, offset, count);
        postFilters[1].process(destinationData, destinationData, bufferSize, offset, count);
        postFilters[2].process(destinationData, destinationData, bufferSize, offset, count);
        postFilters[3].process(destinationData, destinationData, bufferSize, offset, count);
    }
}

void DynamicsCompressor::reset()
{
    m_lastFilterStageRatio = -1;  // for recalc
    m_lastAnchor = -1;
    m_lastFilterStageGain = -1;

    for (unsigned channel = 0; channel < m_numberOfChannels; ++channel)
    {
        for (unsigned stageIndex = 0; stageIndex < 4; ++stageIndex)
        {
            m_preFilterPacks[channel]->filters[stageIndex].reset();
            m_postFilterPacks[channel]->filters[stageIndex].reset();
        }
    }

    m_compressor.reset();
}

void DynamicsCompressor::setNumberOfChannels(unsigned numberOfChannels)
{
    if (m_preFilterPacks.size() == numberOfChannels)
        return;

    m_preFilterPacks.clear();
    m_postFilterPacks.clear();
    for (unsigned i = 0; i < numberOfChannels; ++i)
    {
        m_preFilterPacks.push_back(std::unique_ptr<ZeroPoleFilterPack4>(new ZeroPoleFilterPack4()));
        m_postFilterPacks.push_back(std::unique_ptr<ZeroPoleFilterPack4>(new ZeroPoleFilterPack4()));
    }

    m_sourceChannels = std::unique_ptr<const float * []>(new const float *[numberOfChannels]);
    m_destinationChannels = std::unique_ptr<float * []>(new float *[numberOfChannels]);

    m_compressor.setNumberOfChannels(numberOfChannels);
    m_numberOfChannels = numberOfChannels;
}

double DynamicsCompressor::tailTime(ContextRenderLock & r) const
{
    return 0;
}
double DynamicsCompressor::latencyTime(ContextRenderLock & r) const
{
    return m_compressor.latencyFrames() / static_cast<double>(r.context()->sampleRate());
}

}  // namespace lab
