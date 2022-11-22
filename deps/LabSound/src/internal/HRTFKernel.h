// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef HRTFKernel_h
#define HRTFKernel_h

#include "internal/FFTFrame.h"
#include <vector>

namespace lab
{

class AudioChannel;

// HRTF stands for Head-Related Transfer Function.
// HRTFKernel is a frequency-domain representation of an impulse-response used as part of the spatialized panning system.
// For a given azimuth / elevation angle there will be one HRTFKernel for the left ear transfer function, and one for the right ear.
// The leading delay (average group delay) for each impulse response is extracted:
//      m_fftFrame is the frequency-domain representation of the impulse response with the delay removed
//      m_frameDelay is the leading delay of the original impulse response.
class HRTFKernel
{
public:
    // Note: this is destructive on the passed in AudioChannel.
    // The length of channel must be a power of two.
    HRTFKernel(AudioChannel *, int fftSize, float sampleRate);

    HRTFKernel(std::unique_ptr<FFTFrame> fftFrame, float frameDelay, float sampleRate)
        : m_fftFrame(std::move(fftFrame))
        , m_frameDelay(frameDelay)
        , m_sampleRate(sampleRate)
    {
    }

    FFTFrame * fftFrame() { return m_fftFrame.get(); }

    int fftSize() const { return m_fftFrame->fftSize(); }
    float frameDelay() const { return m_frameDelay; }

    // Converts back into impulse-response form.
    std::unique_ptr<AudioChannel> createImpulseResponse();

    float sampleRate() const { return m_sampleRate; }

private:
    // Note: this is destructive on the passed in AudioChannel.
    std::unique_ptr<FFTFrame> m_fftFrame;
    float m_frameDelay;
    float m_sampleRate;
};

typedef std::vector<std::shared_ptr<HRTFKernel>> HRTFKernelList;

// Given two HRTFKernels, and an interpolation factor x: 0 -> 1, returns an interpolated HRTFKernel.
std::unique_ptr<HRTFKernel> MakeInterpolatedKernel(HRTFKernel * kernel1, HRTFKernel * kernel2, float x);

}  // namespace lab

#endif  // HRTFKernel_h
