// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/AudioBus.h"
#include "LabSound/core/Macros.h"
#include "LabSound/core/WindowFunctions.h"

#include "LabSound/extended/AudioContextLock.h"
#include "LabSound/extended/RealtimeAnalyser.h"
#include "LabSound/extended/Util.h"

#include "internal/Assertions.h"
#include "internal/AudioUtilities.h"
#include "internal/FFTFrame.h"
#include "internal/VectorMath.h"

#include <algorithm>
#include <complex>
#include <limits.h>

using namespace std;

namespace lab
{

const double RealtimeAnalyser::DefaultSmoothingTimeConstant = 0.8;
const double RealtimeAnalyser::DefaultMinDecibels = -100;
const double RealtimeAnalyser::DefaultMaxDecibels = -30;

// All FFT implementations are expected to handle power-of-two sizes MinFFTSize <= size <= MaxFFTSize.
const int RealtimeAnalyser::DefaultFFTSize = 2048;
const int RealtimeAnalyser::MinFFTSize = 32;
const int RealtimeAnalyser::MaxFFTSize = 2048;
const int RealtimeAnalyser::InputBufferSize = RealtimeAnalyser::MaxFFTSize * 2;

RealtimeAnalyser::RealtimeAnalyser(int fftSize)
    : m_inputBuffer(InputBufferSize)
    , m_writeIndex(0)
    , m_smoothingTimeConstant(DefaultSmoothingTimeConstant)
    , m_minDecibels(DefaultMinDecibels)
    , m_maxDecibels(DefaultMaxDecibels)
{
    uint32_t size = max(min(RoundNextPow2(fftSize), MaxFFTSize), MinFFTSize);
    m_fftSize = size;

    m_analysisFrame = std::unique_ptr<FFTFrame>(new FFTFrame(size));

    // m_magnitudeBuffer has size = fftSize / 2 because it contains floats reduced from complex values in m_analysisFrame.
    m_magnitudeBuffer.allocate(size / 2);
}

RealtimeAnalyser::~RealtimeAnalyser() {}

void RealtimeAnalyser::reset()
{
    m_writeIndex = 0;
    m_inputBuffer.zero();
    m_magnitudeBuffer.zero();
}

void RealtimeAnalyser::setFftSize(int fftSize)
{
    m_writeIndex = 0;

    int size = max(min(RoundNextPow2(fftSize), MaxFFTSize), MinFFTSize);
    m_fftSize = size;

    m_analysisFrame = std::unique_ptr<FFTFrame>(new FFTFrame(size));

    m_inputBuffer.zero();
    m_magnitudeBuffer.allocate(size / 2);
}

void RealtimeAnalyser::writeInput(ContextRenderLock & r, AudioBus * bus, int framesToProcess)
{
    bool isBusGood = bus && bus->numberOfChannels() > 0 && bus->channel(0)->length() >= framesToProcess && r.context();
    if (!isBusGood)
        return;

    // FIXME : allow to work with non-FFTSize divisible chunking
    bool isDestinationGood = m_writeIndex < m_inputBuffer.size() && m_writeIndex + framesToProcess <= m_inputBuffer.size();
    ASSERT(isDestinationGood);
    if (!isDestinationGood)
        return;

    // copy data for analysis
    const float * source = bus->channel(0)->data();
    float * dest = m_inputBuffer.data() + m_writeIndex;

    // The source has already been sanity checked with isBusGood above.
    memcpy(dest, source, sizeof(float) * framesToProcess);

    // Sum all channels in one if numberOfChannels > 1.
    const size_t numberOfChannels = bus->numberOfChannels();
    if (numberOfChannels > 1)
    {
        for (int i = 1; i < numberOfChannels; i++)
        {
            source = bus->channel(i)->data();
            VectorMath::vadd(dest, 1, source, 1, dest, 1, framesToProcess);
        }
        const float scale = 1.f / numberOfChannels;
        VectorMath::vsmul(dest, 1, &scale, dest, 1, framesToProcess);
    }

    m_writeIndex += framesToProcess;
    if (m_writeIndex >= InputBufferSize)
        m_writeIndex = 0;
}

void RealtimeAnalyser::doFFTAnalysis()
{
    // Unroll the input buffer into a temporary buffer, where we'll apply an analysis window followed by an FFT.
    uint32_t fftSize = this->fftSize();

    AudioFloatArray temporaryBuffer(fftSize);
    float * inputBuffer = m_inputBuffer.data();
    float * tempP = temporaryBuffer.data();

    // Take the previous fftSize values from the input buffer and copy into the temporary buffer.
    size_t writeIndex = m_writeIndex;
    if (writeIndex < fftSize)
    {
        memcpy(tempP, inputBuffer + writeIndex - fftSize + InputBufferSize, sizeof(*tempP) * (fftSize - writeIndex));
        memcpy(tempP + fftSize - writeIndex, inputBuffer, sizeof(*tempP) * writeIndex);
    }
    else
    {
        memcpy(tempP, inputBuffer + writeIndex - fftSize, sizeof(*tempP) * fftSize);
    }

    // Window the input samples.
    ApplyWindowFunctionInplace(WindowFunction::blackman, tempP, fftSize);

    // Do the analysis.
    m_analysisFrame->computeForwardFFT(tempP);

    float * realP = m_analysisFrame->realData();
    float * imagP = m_analysisFrame->imagData();

    // Erase the packed nyquist component.
    imagP[0] = 0;

    // Normalize so than an input sine wave at 0dBfs registers as 0dBfs (undo FFT scaling factor).
    const double magnitudeScale = 1.0 / DefaultFFTSize;

    // A value of 0 does no averaging with the previous result.  Larger values produce slower, but smoother changes.
    double k = m_smoothingTimeConstant;
    k = max(0.0, k);
    k = min(1.0, k);

    // Convert the analysis data from complex to magnitude and average with the previous result.
    float * destination = magnitudeBuffer().data();
    size_t n = magnitudeBuffer().size();
    for (size_t i = 0; i < n; ++i)
    {
        std::complex<double> c(realP[i], imagP[i]);
        double scalarMagnitude = abs(c) * magnitudeScale;
        destination[i] = float(k * destination[i] + (1 - k) * scalarMagnitude);
    }
}

void RealtimeAnalyser::getFloatFrequencyData(std::vector<float> & destinationArray)
{
    if (!destinationArray.size())
        return;

    doFFTAnalysis();

    // Convert from linear magnitude to floating-point decibels.
    const double minDecibels = m_minDecibels;
    size_t sourceLength = magnitudeBuffer().size();
    size_t len = min(sourceLength, destinationArray.size());
    if (len > 0)
    {
        const float * source = magnitudeBuffer().data();
        for (size_t i = 0; i < len; ++i)
        {
            float linearValue = source[i];
            double dbMag = !linearValue ? minDecibels : AudioUtilities::linearToDecibels(linearValue);
            destinationArray[i] = float(dbMag);
        }
    }
}

void RealtimeAnalyser::getByteFrequencyData(std::vector<uint8_t> & destinationArray, bool resample)
{
    if (!destinationArray.size() || !magnitudeBuffer().size())
        return;

    doFFTAnalysis();

    size_t len = destinationArray.size();
    uint8_t * dest = &destinationArray[0];
    std::vector<uint8_t> result;
    if (destinationArray.size() == frequencyBinCount())
        resample = false;
    else
    {
        if (resample) {
            len = frequencyBinCount();
            result.resize(len);
            dest = &result[0];
        }
    }

    // Convert from linear magnitude to unsigned-byte decibels.
    size_t sourceLength = magnitudeBuffer().size();
    const double rangeScaleFactor = m_maxDecibels == m_minDecibels ? 1 : 1 / (m_maxDecibels - m_minDecibels);
    const double minDecibels = m_minDecibels;

    const float * source = magnitudeBuffer().data();
    for (size_t i = 0; i < len; ++i)
    {
        float linearValue = source[i];
        double dbMag = !linearValue ? minDecibels : AudioUtilities::linearToDecibels(linearValue);

        // The range m_minDecibels to m_maxDecibels will be scaled to byte values from 0 to UCHAR_MAX.
        double scaledValue = UCHAR_MAX * (dbMag - minDecibels) * rangeScaleFactor;

        // Clip to valid range.
        if (scaledValue < 0)
            scaledValue = 0;
        if (scaledValue > UCHAR_MAX)
            scaledValue = UCHAR_MAX;

        dest[i] = static_cast<unsigned char>(scaledValue);
    }
    if (!resample)
        return;

    uint8_t * normalized_data = dest;
    dest = &destinationArray[0];

    size_t src_size = frequencyBinCount();
    size_t dst_size = destinationArray.size();

    if (dst_size > src_size)
    {
        // upsample via linear interpolation
        size_t steps = dst_size;
        float u_step = 1.f / static_cast<float>(steps);
        float u = 0;
        for (size_t step = 0; step < steps; ++step, u += u_step)
        {
            float t = u * src_size;
            size_t u0 = static_cast<size_t>(t);
            t = t - static_cast<float>(u0);
            dest[step] = static_cast<uint8_t>(normalized_data[u0] * t + normalized_data[u0 + 1] * (1.f - t));
        }
    }
    else
    {
        // down sample by taking the max of accumulated bins, stepped across using Bresenham.
        // @cbb a convolution like lanczos would be better if the bins are to be treated as splines
        float d_src = static_cast<float>(src_size);
        float d_dst = static_cast<float>(dst_size);
        float d_err = d_dst / d_src;
        memset(&dest[0], 0, dst_size);
        size_t u = 0;
        float error = 0;
        for (size_t step = 0; step < src_size - 1 && u < dst_size; ++step)
        {
            dest[u] = std::max(dest[u], normalized_data[step]);
            error += d_err;
            if (error > 0.5f)
            {
                ++u;
                error -= 1.f;
            }
            ASSERT(u < src_size);
        }
    }
}

// LabSound begin
void RealtimeAnalyser::getFloatTimeDomainData(std::vector<float> & destinationArray)
{
    if (!destinationArray.size())
        return;

    size_t fftSize = this->fftSize();
    size_t len = min(fftSize, destinationArray.size());
    if (len > 0)
    {
        bool isInputBufferGood = m_inputBuffer.size() == InputBufferSize && m_inputBuffer.size() > fftSize;
        ASSERT(isInputBufferGood);
        if (!isInputBufferGood)
            return;

        float * inputBuffer = m_inputBuffer.data();
        size_t writeIndex = m_writeIndex;

        for (size_t i = 0; i < len; ++i)
            // Buffer access is protected due to modulo operation.
            destinationArray[i] = inputBuffer[(i + writeIndex - fftSize + InputBufferSize) % InputBufferSize];
    }
}
// LabSound end

void RealtimeAnalyser::getByteTimeDomainData(std::vector<uint8_t> & destinationArray)
{
    if (!destinationArray.size())
        return;

    size_t fftSize = this->fftSize();
    size_t len = min(fftSize, destinationArray.size());
    if (len > 0)
    {
        bool isInputBufferGood = m_inputBuffer.size() == InputBufferSize && m_inputBuffer.size() > fftSize;
        ASSERT(isInputBufferGood);
        if (!isInputBufferGood)
            return;

        float * inputBuffer = m_inputBuffer.data();
        size_t writeIndex = m_writeIndex;

        for (size_t i = 0; i < len; ++i)
        {
            // Buffer access is protected due to modulo operation.
            float value = inputBuffer[(i + writeIndex - fftSize + InputBufferSize) % InputBufferSize];

            // Scale from nominal -1 -> +1 to unsigned byte.
            double scaledValue = AudioNode::ProcessingSizeInFrames * (value + 1);

            // Clip to valid range.
            if (scaledValue < 0)
                scaledValue = 0;
            if (scaledValue > UCHAR_MAX)
                scaledValue = UCHAR_MAX;

            destinationArray[i] = static_cast<unsigned char>(scaledValue);
        }
    }
}

}  // namespace lab
