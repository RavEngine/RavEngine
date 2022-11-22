// License: BSD 3 Clause
// Copyright (C) 2011, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef DynamicsCompressorKernel_h
#define DynamicsCompressorKernel_h

#include "LabSound/core/AudioArray.h"

#include <memory>
#include <vector>

namespace lab
{

class ContextRenderLock;

class DynamicsCompressorKernel
{

public:
    DynamicsCompressorKernel(unsigned numberOfChannels);

    void setNumberOfChannels(unsigned);

    // Performs stereo-linked compression.
    void process(ContextRenderLock &,
                 const float * sourceChannels[],
                 float * destinationChannels[],
                 unsigned numberOfChannels,
                 int framesToProcess,

                 float dbThreshold,
                 float dbKnee,
                 float ratio,
                 float attackTime,
                 float releaseTime,
                 float preDelayTime,
                 float dbPostGain,
                 float effectBlend,

                 float releaseZone1,
                 float releaseZone2,
                 float releaseZone3,
                 float releaseZone4);

    void reset();

    unsigned latencyFrames() const { return m_lastPreDelayFrames; }

    float meteringGain() const { return m_meteringGain; }

protected:
    float m_detectorAverage;
    float m_compressorGain;

    // Metering
    float m_meteringReleaseK;
    float m_meteringGain;

    // Lookahead section.
    enum
    {
        MaxPreDelayFrames = 1024
    };
    enum
    {
        MaxPreDelayFramesMask = MaxPreDelayFrames - 1
    };
    enum
    {
        DefaultPreDelayFrames = 256
    };  // setPreDelayTime() will override this initial value
    unsigned m_lastPreDelayFrames;
    void setPreDelayTime(float time, float sampleRate);

    std::vector<std::unique_ptr<AudioFloatArray>> m_preDelayBuffers;

    int m_preDelayReadIndex;
    int m_preDelayWriteIndex;

    float m_maxAttackCompressionDiffDb;

    // Static compression curve.
    float kneeCurve(float x, float k);
    float saturate(float x, float k);
    float slopeAt(float x, float k);
    float kAtSlope(float desiredSlope);

    float updateStaticCurveParameters(float dbThreshold, float dbKnee, float ratio);

    // Amount of input change in dB required for 1 dB of output change.
    // This applies to the portion of the curve above m_kneeThresholdDb (see below).
    float m_ratio;
    float m_slope;  // Inverse ratio.

    // The input to output change below the threshold is linear 1:1.
    float m_linearThreshold;
    float m_dbThreshold;

    // m_dbKnee is the number of dB above the threshold before we enter the "ratio" portion of the curve.
    // m_kneeThresholdDb = m_dbThreshold + m_dbKnee
    // The portion between m_dbThreshold and m_kneeThresholdDb is the "soft knee" portion of the curve
    // which transitions smoothly from the linear portion to the ratio portion.
    float m_dbKnee;
    float m_kneeThreshold;
    float m_kneeThresholdDb;
    float m_ykneeThresholdDb;

    // Internal parameter for the knee portion of the curve.
    float m_K;
};

}  // namespace lab

#endif  // DynamicsCompressorKernel_h
