// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef FFTFrame_h
#define FFTFrame_h

#include "LabSound/core/AudioArray.h"

#include "LabSound/core/Macros.h"

#include <memory>
#include <vector>

#if defined(LABSOUND_PLATFORM_OSX) && !defined(USE_KISS_FFT)
  #define USE_ACCELERATE_FFT 1
#else
  #define USE_ACCELERATE_FFT 0
#endif

#if USE_ACCELERATE_FFT
  #include <Accelerate/Accelerate.h>
#endif 

#if defined(USE_KISS_FFT)
#include <kissfft/kiss_fft.hpp>
#include <kissfft/kiss_fftr.hpp>
#endif

namespace lab
{

// Defines the interface for an "FFT frame", an object which is able to perform a forward
// and reverse FFT, internally storing the resultant frequency-domain data.
class FFTFrame
{

public:
    FFTFrame();  // creates a blank/empty frame for later use with createInterpolatedFrame()
    FFTFrame(int fftSize);

    // Copy
    FFTFrame(const FFTFrame & frame);
    ~FFTFrame();

    void computeForwardFFT(const float * data);
    void computeInverseFFT(float * data);
    void multiply(const FFTFrame & frame);  // multiplies ourself with frame : effectively operator*=()

    float * realData() const;
    float * imagData() const;

    void print();  // for debugging

    // The remaining public methods have cross-platform implementations:

    // Interpolates from frame1 -> frame2 as x goes from 0.0 -> 1.0
    static std::unique_ptr<FFTFrame> createInterpolatedFrame(const FFTFrame & frame1, const FFTFrame & frame2, double x);

    void doPaddedFFT(const float * data, int dataSize);  // zero-padding with dataSize <= fftSize
    double extractAverageGroupDelay();
    void addConstantGroupDelay(double sampleFrameDelay);

    int fftSize() const { return m_FFTSize; }
    int log2FFTSize() const { return m_log2FFTSize; }

#if USE_ACCELERATE_FFT
    static void cleanup();
#endif

private:
    int m_FFTSize;
    int m_log2FFTSize;

    void interpolateFrequencyComponents(const FFTFrame & frame1, const FFTFrame & frame2, double x);

#if USE_ACCELERATE_FFT
    DSPSplitComplex & dspSplitComplex()
    {
        return m_frame;
    }
    DSPSplitComplex dspSplitComplex() const { return m_frame; }

    static FFTSetup fftSetupForSize(int fftSize);

    static FFTSetup * fftSetups;

    FFTSetup m_FFTSetup;

    DSPSplitComplex m_frame;
    AudioFloatArray m_realData;
    AudioFloatArray m_imagData;
#endif

#if defined(USE_KISS_FFT)
    kiss_fftr_cfg mFFT;
    kiss_fftr_cfg mIFFT;

    kiss_fft_cpx * m_cpxInputData;
    kiss_fft_cpx * m_cpxOutputData;

    std::vector<kiss_fft_cpx> mOutputBuffer;

    AudioFloatArray m_realData;
    AudioFloatArray m_imagData;
#endif

};

}  // namespace lab

#endif  // FFTFrame_h
