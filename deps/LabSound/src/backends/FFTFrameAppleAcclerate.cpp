// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/Macros.h"

#if defined(LABSOUND_PLATFORM_OSX) && !defined(WEBAUDIO_KISSFFT)

#include "internal/Assertions.h"
#include "internal/FFTFrame.h"
#include "internal/VectorMath.h"

namespace lab
{

const int kMaxFFTPow2Size = 24;

FFTSetup * FFTFrame::fftSetups = 0;

// Normal constructor: allocates for a given fftSize
FFTFrame::FFTFrame(int fftSize)
    : m_realData(fftSize)
    , m_imagData(fftSize)
{
    m_FFTSize = fftSize;
    m_log2FFTSize = static_cast<size_t>(log2(fftSize));

    // We only allow power of two
    ASSERT(1UL << m_log2FFTSize == m_FFTSize);

    // Lazily create and share fftSetup with other frames
    m_FFTSetup = fftSetupForSize(fftSize);

    // Setup frame data
    m_frame.realp = m_realData.data();
    m_frame.imagp = m_imagData.data();
}

// Creates a blank/empty frame (interpolate() must later be called)
FFTFrame::FFTFrame()
    : m_realData(0)
    , m_imagData(0)
{
    // Later will be set to correct values when interpolate() is called
    m_frame.realp = 0;
    m_frame.imagp = 0;

    m_FFTSize = 0;
    m_log2FFTSize = 0;
}

// Copy constructor
FFTFrame::FFTFrame(const FFTFrame & frame)
    : m_FFTSize(frame.m_FFTSize)
    , m_log2FFTSize(frame.m_log2FFTSize)
    , m_FFTSetup(frame.m_FFTSetup)
    , m_realData(frame.m_FFTSize)
    , m_imagData(frame.m_FFTSize)
{
    // Setup frame data
    m_frame.realp = m_realData.data();
    m_frame.imagp = m_imagData.data();

    // Copy/setup frame data
    size_t nbytes = sizeof(float) * m_FFTSize;
    memcpy(realData(), frame.m_frame.realp, nbytes);
    memcpy(imagData(), frame.m_frame.imagp, nbytes);
}

FFTFrame::~FFTFrame()
{
}

void FFTFrame::cleanup()
{
    if (!fftSetups)
        return;

    for (int i = 0; i < kMaxFFTPow2Size; ++i)
    {
        if (fftSetups[i])
            vDSP_destroy_fftsetup(fftSetups[i]);
    }

    free(fftSetups);
    fftSetups = 0;
}

void FFTFrame::multiply(const FFTFrame & frame)
{
    FFTFrame & frame1 = *this;
    const FFTFrame & frame2 = frame;

    float * realP1 = frame1.realData();
    float * imagP1 = frame1.imagData();
    const float * realP2 = frame2.realData();
    const float * imagP2 = frame2.imagData();

    size_t halfSize = m_FFTSize / 2;
    float real0 = realP1[0];
    float imag0 = imagP1[0];

    // Complex multiply
    VectorMath::zvmul(realP1, imagP1, realP2, imagP2, realP1, imagP1, halfSize);

    // Multiply the packed DC/nyquist component
    realP1[0] = real0 * realP2[0];
    imagP1[0] = imag0 * imagP2[0];

    // Scale accounts for vecLib's peculiar scaling
    // This ensures the right scaling all the way back to inverse FFT
    float scale = 0.5f;

    VectorMath::vsmul(realP1, 1, &scale, realP1, 1, halfSize);
    VectorMath::vsmul(imagP1, 1, &scale, imagP1, 1, halfSize);
}

void FFTFrame::computeForwardFFT(const float * data)
{
    vDSP_ctoz((DSPComplex *) data, 2, &m_frame, 1, m_FFTSize / 2);
    vDSP_fft_zrip(m_FFTSetup, &m_frame, 1, m_log2FFTSize, FFT_FORWARD);
}

void FFTFrame::computeInverseFFT(float * data)
{
    vDSP_fft_zrip(m_FFTSetup, &m_frame, 1, m_log2FFTSize, FFT_INVERSE);
    vDSP_ztoc(&m_frame, 1, (DSPComplex *) data, 2, m_FFTSize / 2);

    // Do final scaling so that x == IFFT(FFT(x))
    float scale = 0.5f / m_FFTSize;
    vDSP_vsmul(data, 1, &scale, data, 1, m_FFTSize);
}

FFTSetup FFTFrame::fftSetupForSize(int fftSize)
{
    if (!fftSetups)
    {
        fftSetups = (FFTSetup *) malloc(sizeof(FFTSetup) * kMaxFFTPow2Size);
        memset(fftSetups, 0, sizeof(FFTSetup) * kMaxFFTPow2Size);
    }

    int pow2size = static_cast<int>(log2(fftSize));
    ASSERT(pow2size < kMaxFFTPow2Size);
    if (!fftSetups[pow2size])
        fftSetups[pow2size] = vDSP_create_fftsetup(pow2size, FFT_RADIX2);

    return fftSetups[pow2size];
}

float * FFTFrame::realData() const
{
    return m_frame.realp;
}

float * FFTFrame::imagData() const
{
    return m_frame.imagp;
}

}  // namespace lab

#endif  // #if OS(DARWIN)
