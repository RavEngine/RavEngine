// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef VectorMath_h
#define VectorMath_h

#include <cstddef>

// Defines the interface for several vector math functions whose implementation will ideally be optimized.

namespace lab
{

namespace VectorMath
{
    // Vector scalar multiply and then add.
    void vsma(const float * sourceP, int sourceStride, const float * scale, float * destP, int destStride, int framesToProcess);

    void vsmul(const float * sourceP, int sourceStride, const float * scale, float * destP, int destStride, int framesToProcess);
    void vadd(const float * source1P, int sourceStride1, const float * source2P, int sourceStride2, float * destP, int destStride, int framesToProcess);
    void vintlve(const float * realSrcP, const float * imagSrcP, float * destP, int framesToProcess);  // for KissFFT
    void vdeintlve(const float * sourceP, float * realDestP, float * imagDestP, int framesToProcess);  // for KissFFT

    // Finds the maximum magnitude of a float vector.
    void vmaxmgv(const float * sourceP, int sourceStride, float * maxP, int framesToProcess);

    // Sums the squares of a float vector's elements.
    void vsvesq(const float * sourceP, int sourceStride, float * sumP, int framesToProcess);

    // For an element-by-element multiply of two float vectors.
    void vmul(const float * source1P, int sourceStride1, const float * source2P, int sourceStride2, float * destP, int destStride, int framesToProcess);

    // Multiplies two complex vectors.
    void zvmul(const float * real1P, const float * imag1P, const float * real2P, const float * imag2P, float * realDestP, float * imagDestP, int framesToProcess);

    // Copies elements while clipping values to the threshold inputs.
    void vclip(const float * sourceP, int sourceStride, const float * lowThresholdP, const float * highThresholdP, float * destP, int destStride, int framesToProcess);

}  // namespace VectorMath

}  // namespace lab

#endif  // VectorMath_h
