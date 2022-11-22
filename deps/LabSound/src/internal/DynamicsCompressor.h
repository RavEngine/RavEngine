// License: BSD 3 Clause
// Copyright (C) 2011, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef DynamicsCompressor_h
#define DynamicsCompressor_h

#include "LabSound/core/AudioArray.h"

#include "internal/DynamicsCompressorKernel.h"
#include "internal/ZeroPole.h"

#include <vector>

namespace lab
{

class AudioBus;

// DynamicsCompressor implements a flexible audio dynamics compression effect such as
// is commonly used in musical production and game audio. It lowers the volume
// of the loudest parts of the signal and raises the volume of the softest parts,
// making the sound richer, fuller, and more controlled.

class DynamicsCompressor
{

public:
    enum
    {
        ParamThreshold,
        ParamKnee,
        ParamRatio,
        ParamAttack,
        ParamRelease,
        ParamPreDelay,
        ParamReleaseZone1,
        ParamReleaseZone2,
        ParamReleaseZone3,
        ParamReleaseZone4,
        ParamPostGain,
        ParamFilterStageGain,
        ParamFilterStageRatio,
        ParamFilterAnchor,
        ParamEffectBlend,
        ParamReduction,
        ParamLast
    };

    DynamicsCompressor(unsigned numberOfChannels);

    void process(ContextRenderLock &, const AudioBus * sourceBus, AudioBus * destinationBus, int bufferSize, int offset, int count);
    void reset();
    void setNumberOfChannels(unsigned);

    void setParameterValue(unsigned parameterID, float value);
    float parameterValue(unsigned parameterID);

    double tailTime(ContextRenderLock & r) const;
    double latencyTime(ContextRenderLock & r) const;

protected:
    unsigned m_numberOfChannels;

    // m_parameters holds the tweakable compressor parameters.
    float m_parameters[ParamLast];
    void initializeParameters();

    // Emphasis filter controls.
    float m_lastFilterStageRatio;
    float m_lastAnchor;
    float m_lastFilterStageGain;

    typedef struct
    {
        ZeroPole filters[4];
    } ZeroPoleFilterPack4;

    // Per-channel emphasis filters.
    std::vector<std::unique_ptr<ZeroPoleFilterPack4>> m_preFilterPacks;
    std::vector<std::unique_ptr<ZeroPoleFilterPack4>> m_postFilterPacks;

    std::unique_ptr<const float * []> m_sourceChannels;
    std::unique_ptr<float * []> m_destinationChannels;

    void setEmphasisStageParameters(unsigned stageIndex, float gain, float normalizedFrequency /* 0 -> 1 */);
    void setEmphasisParameters(float gain, float anchorFreq, float filterStageRatio);

    // The core compressor.
    DynamicsCompressorKernel m_compressor;
};

}  // namespace lab

#endif  // DynamicsCompressor_h
