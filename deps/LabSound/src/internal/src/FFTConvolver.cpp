// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "internal/FFTConvolver.h"
#include "internal/Assertions.h"
#include "internal/VectorMath.h"

namespace lab
{

using namespace VectorMath;

FFTConvolver::FFTConvolver(int fftSize)
    : m_frame(fftSize)
    , m_readWriteIndex(0)
    , m_inputBuffer(fftSize)  // 2nd half of buffer is always zeroed
    , m_outputBuffer(fftSize)
    , m_lastOverlapBuffer(fftSize / 2)
{
}

void FFTConvolver::process(FFTFrame * fftKernel, const float * sourceP, float * destP, int framesToProcess)
{
    int halfSize = fftSize() / 2;

    // framesToProcess must be an exact multiple of halfSize,
    // or halfSize is a multiple of framesToProcess when halfSize > framesToProcess.
    bool isGood = !(halfSize % framesToProcess && framesToProcess % halfSize);
    ASSERT(isGood);

    if (!isGood)
        return;

    int numberOfDivisions = halfSize <= framesToProcess ? (framesToProcess / halfSize) : 1;
    int divisionSize = numberOfDivisions == 1 ? framesToProcess : halfSize;

    for (int i = 0; i < numberOfDivisions; ++i, sourceP += divisionSize, destP += divisionSize)
    {
        // Copy samples to input buffer (note contraint above!)
        float * inputP = m_inputBuffer.data();

        // Sanity check
        bool isCopyGood1 = sourceP && inputP && m_readWriteIndex + divisionSize <= m_inputBuffer.size();
        ASSERT(isCopyGood1);
        if (!isCopyGood1)
            return;

        memcpy(inputP + m_readWriteIndex, sourceP, sizeof(float) * divisionSize);

        // Copy samples from output buffer
        float * outputP = m_outputBuffer.data();

        // Sanity check
        bool isCopyGood2 = destP && outputP && m_readWriteIndex + divisionSize <= m_outputBuffer.size();
        ASSERT(isCopyGood2);
        if (!isCopyGood2)
            return;

        memcpy(destP, outputP + m_readWriteIndex, sizeof(float) * divisionSize);
        m_readWriteIndex += divisionSize;

        // Check if it's time to perform the next FFT
        if (m_readWriteIndex == halfSize)
        {

            // The input buffer is now filled (get frequency-domain version)
            m_frame.computeForwardFFT(m_inputBuffer.data());
            m_frame.multiply(*fftKernel);
            m_frame.computeInverseFFT(m_outputBuffer.data());

            // Overlap-add 1st half from previous time
            vadd(m_outputBuffer.data(), 1, m_lastOverlapBuffer.data(), 1, m_outputBuffer.data(), 1, halfSize);

            // Finally, save 2nd half of result
            bool isCopyGood3 = m_outputBuffer.size() == 2 * halfSize && m_lastOverlapBuffer.size() == halfSize;
            ASSERT(isCopyGood3);
            if (!isCopyGood3)
                return;

            memcpy(m_lastOverlapBuffer.data(), m_outputBuffer.data() + halfSize, sizeof(float) * halfSize);

            // Reset index back to start for next time
            m_readWriteIndex = 0;
        }
    }
}

void FFTConvolver::reset()
{
    m_lastOverlapBuffer.zero();
    m_readWriteIndex = 0;
}

}  // namespace lab
