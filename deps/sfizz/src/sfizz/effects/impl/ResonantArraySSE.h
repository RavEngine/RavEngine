// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "ResonantArray.h"
#include "ResonantStringSSE.h"
#include "Buffer.h"
#include "SIMDConfig.h"

#if SFIZZ_CPU_FAMILY_X86_64 || SFIZZ_CPU_FAMILY_I386
namespace sfz {
namespace fx {

class ResonantArraySSE final : public ResonantArray {
public:
    ResonantArraySSE();
    ~ResonantArraySSE();

    void setup(
        float sampleRate, unsigned numStrings,
        const float pitches[], const float bandwidths[],
        const float feedbacks[], const float gains[]) override;

    void setSamplesPerBlock(unsigned samplesPerBlock) override;

    void clear() override;

    void process(const float *inPtr, float *outPtr, unsigned numFrames) override;

private:
    Buffer<ResonantStringSSE, 16> _stringPacks;
    unsigned _numStrings = 0;
    Buffer<float, 16> _workBuffer;
};

} // namespace sfz
} // namespace fx
#endif
