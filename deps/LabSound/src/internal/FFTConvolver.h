// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef FFTConvolver_h
#define FFTConvolver_h

#include "LabSound/core/AudioArray.h"

#include "internal/FFTFrame.h"

namespace lab
{

class FFTConvolver
{
public:
    // fftSize must be a power of two
    FFTConvolver(int fftSize);

    // For now, with multiple calls to Process(), framesToProcess MUST add up EXACTLY to fftSize / 2
    //
    // FIXME: Later, we can do more sophisticated buffering to relax this requirement...
    //
    // The input to output latency is equal to fftSize / 2
    //
    // Processing in-place is allowed...
    void process(FFTFrame * fftKernel, const float * sourceP, float * destP, int framesToProcess);

    void reset();

    int fftSize() const { return m_frame.fftSize(); }

private:
    FFTFrame m_frame;

    // Buffer input until we get fftSize / 2 samples then do an FFT
    size_t m_readWriteIndex;
    AudioFloatArray m_inputBuffer;

    // Stores output which we read a little at a time
    AudioFloatArray m_outputBuffer;

    // Saves the 2nd half of the FFT buffer, so we can do an overlap-add with the 1st half of the next one
    AudioFloatArray m_lastOverlapBuffer;
};

}  // namespace lab

#endif  // FFTConvolver_h
