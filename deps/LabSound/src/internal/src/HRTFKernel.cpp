// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "internal/HRTFKernel.h"
#include "internal/Assertions.h"
#include "internal/Biquad.h"
#include "internal/FFTFrame.h"

#include "LabSound/core/AudioChannel.h"

#include "LabSound/core/Macros.h"

#include <algorithm>

using namespace std;

namespace lab
{

// Takes the input AudioChannel as an input impulse response and calculates the average group delay.
// This represents the initial delay before the most energetic part of the impulse response.
// The sample-frame delay is removed from the impulseP impulse response, and this value  is returned.
// the length of the passed in AudioChannel must be a power of 2.
inline float ExtractAverageGroupDelay(AudioChannel * channel, int analysisFFTSize)
{
    ASSERT(channel);

    float * impulseP = channel->mutableData();

    bool isSizeGood = channel->length() >= analysisFFTSize;
    ASSERT(isSizeGood);
    if (!isSizeGood)
        return 0;

    // Check for power-of-2.
    ASSERT(uint64_t(1) << static_cast<uint64_t>(log2(analysisFFTSize)) == analysisFFTSize);

    FFTFrame estimationFrame(analysisFFTSize);
    estimationFrame.computeForwardFFT(impulseP);

    float frameDelay = static_cast<float>(estimationFrame.extractAverageGroupDelay());
    estimationFrame.computeInverseFFT(impulseP);

    return frameDelay;
}

HRTFKernel::HRTFKernel(AudioChannel * channel, int fftSize, float sampleRate)
    : m_frameDelay(0)
    , m_sampleRate(sampleRate)
{
    ASSERT(channel);

    // Determine the leading delay (average group delay) for the response.
    m_frameDelay = ExtractAverageGroupDelay(channel, fftSize / 2);

    float * impulseResponse = channel->mutableData();
    int responseLength = channel->length();

    // We need to truncate to fit into 1/2 the FFT size (with zero padding) in order to do proper convolution.
    int truncatedResponseLength = std::min(responseLength, fftSize / 2);  // truncate if necessary to max impulse response length allowed by FFT

    // Quick fade-out (apply window) at truncation point
    int numberOfFadeOutFrames = static_cast<unsigned>(sampleRate / 4410);  // 10 sample-frames @44.1KHz sample-rate
    ASSERT(numberOfFadeOutFrames < truncatedResponseLength);

    if (numberOfFadeOutFrames < truncatedResponseLength)
    {
        for (int i = truncatedResponseLength - numberOfFadeOutFrames; i < truncatedResponseLength; ++i)
        {
            float x = 1.0f - static_cast<float>(i - (truncatedResponseLength - numberOfFadeOutFrames)) / numberOfFadeOutFrames;
            impulseResponse[i] *= x;
        }
    }

    m_fftFrame = std::unique_ptr<FFTFrame>(new FFTFrame(fftSize));
    m_fftFrame->doPaddedFFT(impulseResponse, truncatedResponseLength);
}

std::unique_ptr<AudioChannel> HRTFKernel::createImpulseResponse()
{
    std::unique_ptr<AudioChannel> channel(new AudioChannel(fftSize()));
    FFTFrame fftFrame(*m_fftFrame);

    // Add leading delay back in.
    fftFrame.addConstantGroupDelay(m_frameDelay);
    fftFrame.computeInverseFFT(channel->mutableData());

    return channel;
}

// Interpolates two kernels with x: 0 -> 1 and returns the result.
std::unique_ptr<HRTFKernel> MakeInterpolatedKernel(HRTFKernel * kernel1, HRTFKernel * kernel2, float x)
{
    ASSERT(kernel1 && kernel2);
    if (!kernel1 || !kernel2)
        return 0;

    ASSERT(x >= 0.0 && x < 1.0);
    x = std::min(1.0f, std::max(0.0f, x));

    float sampleRate1 = kernel1->sampleRate();
    float sampleRate2 = kernel2->sampleRate();

    ASSERT(sampleRate1 == sampleRate2);
    if (sampleRate1 != sampleRate2)
        return 0;

    float frameDelay = (1 - x) * kernel1->frameDelay() + x * kernel2->frameDelay();

    std::unique_ptr<FFTFrame> interpolatedFrame = FFTFrame::createInterpolatedFrame(*kernel1->fftFrame(), *kernel2->fftFrame(), x);
    return std::unique_ptr<HRTFKernel>(new HRTFKernel(std::move(interpolatedFrame), frameDelay, sampleRate1));
}

}  // namespace lab
