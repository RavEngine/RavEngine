// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef HRTFPanner_h
#define HRTFPanner_h

#include "internal/DelayDSPKernel.h"
#include "internal/FFTConvolver.h"
#include "internal/Panner.h"

namespace lab
{

class HRTFPanner : public Panner
{

public:
    HRTFPanner(const float sampleRate);
    virtual ~HRTFPanner();

    // Panner
    virtual void pan(ContextRenderLock &, double azimuth, double elevation, const AudioBus * inputBus, AudioBus * outputBus, int framesToProcess) override;
    virtual void reset() override;

    uint32_t fftSize() const { return fftSizeForSampleRate(m_sampleRate); }
    static uint32_t fftSizeForSampleRate(float sampleRate);

    virtual double tailTime(ContextRenderLock & r) const override;
    virtual double latencyTime(ContextRenderLock & r) const override;

private:
    // Given an azimuth angle in the range -180 -> +180, returns the corresponding azimuth index for the database,
    // and azimuthBlend which is an interpolation value from 0 -> 1.
    int calculateDesiredAzimuthIndexAndBlend(double azimuth, double & azimuthBlend);

    // We maintain two sets of convolvers for smooth cross-faded interpolations when
    // then azimuth and elevation are dynamically changing.
    // When the azimuth and elevation are not changing, we simply process with one of the two sets.
    // Initially we use CrossfadeSelection1 corresponding to m_convolverL1 and m_convolverR1.
    // Whenever the azimuth or elevation changes, a crossfade is initiated to transition
    // to the new position. So if we're currently processing with CrossfadeSelection1, then
    // we transition to CrossfadeSelection2 (and vice versa).
    // If we're in the middle of a transition, then we wait until it is complete before
    // initiating a new transition.

    // Selects either the convolver set (m_convolverL1, m_convolverR1) or (m_convolverL2, m_convolverR2).
    enum CrossfadeSelection
    {
        CrossfadeSelection1,
        CrossfadeSelection2
    };

    CrossfadeSelection m_crossfadeSelection;

    // azimuth/elevation for CrossfadeSelection1.
    int m_azimuthIndex1;
    double m_elevation1;

    // azimuth/elevation for CrossfadeSelection2.
    int m_azimuthIndex2;
    double m_elevation2;

    // A crossfade value 0 <= m_crossfadeX <= 1.
    float m_crossfadeX;

    // Per-sample-frame crossfade value increment.
    float m_crossfadeIncr;

    FFTConvolver m_convolverL1;
    FFTConvolver m_convolverR1;
    FFTConvolver m_convolverL2;
    FFTConvolver m_convolverR2;

    DelayDSPKernel m_delayLineL;
    DelayDSPKernel m_delayLineR;

    AudioFloatArray m_tempL1;
    AudioFloatArray m_tempR1;
    AudioFloatArray m_tempL2;
    AudioFloatArray m_tempR2;
};

}  // namespace lab

#endif  // HRTFPanner_h
