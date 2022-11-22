// License: BSD 2 Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/Macros.h"
#include "internal/Assertions.h"

#if defined(USE_KISS_FFT)

#include "internal/FFTFrame.h"
#include "internal/VectorMath.h"

#include <iostream>
#include <kissfft/kiss_fftr.hpp>

// To use this implementation, add USE_KISS_FFT=1 to the list of preprocessor defines
namespace lab
{

const int kMaxFFTPow2Size = 24;

// Normal constructor: allocates for a given fftSize.
FFTFrame::FFTFrame(int fftSize)
    : m_FFTSize(fftSize), m_log2FFTSize(static_cast<int>(log2((double) fftSize)))
    , mFFT(0), mIFFT(0), m_realData(fftSize / 2 + 1), m_imagData(fftSize / 2 + 1)
{
    // We only allow power of two.
    ASSERT(1UL << m_log2FFTSize == m_FFTSize);

    const int mBinSize = m_FFTSize / 2 + 1;

    mFFT = kiss_fftr_alloc(m_FFTSize, 0, nullptr, nullptr);
    mIFFT = kiss_fftr_alloc(m_FFTSize, 1, nullptr, nullptr);

    m_cpxInputData = new kiss_fft_cpx[mBinSize];
    m_cpxOutputData = new kiss_fft_cpx[mBinSize];

    int nbytes = sizeof(float) * mBinSize;

    memset(realData(), 0, nbytes);
    memset(imagData(), 0, nbytes);
}

// Creates a blank/empty frame (interpolate() must later be called).
FFTFrame::FFTFrame() : m_FFTSize(0), m_log2FFTSize(0), mFFT(0), mIFFT(0)
{
}

// Copy constructor.
FFTFrame::FFTFrame(const FFTFrame & frame) 
    : m_FFTSize(frame.m_FFTSize), m_log2FFTSize(frame.m_log2FFTSize), mFFT(0), mIFFT(0), m_realData(frame.m_FFTSize / 2), m_imagData(frame.m_FFTSize / 2)
{
    mFFT = kiss_fftr_alloc(m_FFTSize, 0, nullptr, nullptr);
    mIFFT = kiss_fftr_alloc(m_FFTSize, 1, nullptr, nullptr);

    const int mBinSize = m_FFTSize / 2 + 1;

    // Copy/setup frame data.
    unsigned nbytes = sizeof(float) * mBinSize;

    memcpy(realData(), frame.realData(), nbytes);
    memcpy(imagData(), frame.imagData(), nbytes);

    m_cpxInputData = new kiss_fft_cpx[mBinSize];
    m_cpxOutputData = new kiss_fft_cpx[mBinSize];
}

FFTFrame::~FFTFrame()
{
    KISS_FFT_FREE(mFFT);
    KISS_FFT_FREE(mIFFT);
    delete m_cpxInputData;
    delete m_cpxOutputData;
}

void FFTFrame::multiply(const FFTFrame & frame)
{
    FFTFrame & frame1 = *this;
    FFTFrame & frame2 = const_cast<FFTFrame &>(frame);

    float * realP1 = frame1.realData();
    float * imagP1 = frame1.imagData();
    const float * realP2 = frame2.realData();
    const float * imagP2 = frame2.imagData();

    unsigned halfSize = fftSize() / 2;
    float real0 = realP1[0];
    float imag0 = imagP1[0];
    VectorMath::zvmul(realP1, imagP1, realP2, imagP2, realP1, imagP1, halfSize);

    // Multiply the packed DC/nyquist component
    realP1[0] = real0 * realP2[0];
    imagP1[0] = imag0 * imagP2[0];
}

void FFTFrame::computeForwardFFT(const float * data)
{
    kiss_fftr(mFFT, data, m_cpxOutputData);

    float * outputData = reinterpret_cast<float *>(m_cpxOutputData);  // interleaved .r / .i

    // De-interleave to separate real and complex arrays.
    VectorMath::vdeintlve(outputData, m_realData.data(), m_imagData.data(), m_FFTSize);
}

void FFTFrame::computeInverseFFT(float * data)
{
    const uint32_t inputSize = m_FFTSize / 2 + 1;

    for (uint32_t i = 0; i < inputSize; ++i)
    {
        m_cpxInputData[i].r = m_realData.data()[i];
        m_cpxInputData[i].i = m_imagData.data()[i];
    }

    // Inverse-transform the (inputSize) points of data in each
    // of (m_cpxInputData.r) and (m_cpxInputData.i)
    float * outData = reinterpret_cast<float *>(m_cpxOutputData);  // .r + .i
    kiss_fftri(mIFFT, m_cpxInputData, outData);

    // Scale so that a forward then inverse FFT yields exactly the original data and
    // store the resulting (m_FFTSize) points in (data).
    //  x == IFFT(FFT(x))
    const float scale = 1.0f / m_FFTSize;
    VectorMath::vsmul(outData, 1, &scale, data, 1, m_FFTSize);
}

float * FFTFrame::realData() const
{
    return const_cast<float *>(m_realData.data());
}

float * FFTFrame::imagData() const
{
    return const_cast<float *>(m_imagData.data());
}

}  // namespace lab

#endif  // USE KISS FFT
