// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include <memory>

namespace sfz {
namespace fx {

class ResonantString;

//------------------------------------------------------------------------------

class ResonantArray {
public:
    virtual ~ResonantArray() {}

    virtual void setup(
        float sampleRate, unsigned numStrings,
        const float pitches[], const float bandwidths[],
        const float feedbacks[], const float gains[]) = 0;

    virtual void setSamplesPerBlock(unsigned samplesPerBlock) = 0;

    virtual void clear() = 0;

    virtual void process(const float *input, float *output, unsigned numFrames) = 0;
};

//------------------------------------------------------------------------------

class ResonantArrayScalar final : public ResonantArray {
public:
    ResonantArrayScalar();
    ~ResonantArrayScalar();

    void setup(
        float sampleRate, unsigned numStrings,
        const float pitches[], const float bandwidths[],
        const float feedbacks[], const float gains[]) override;

    void setSamplesPerBlock(unsigned) override {}

    void clear() override;

    void process(const float *inPtr, float *outPtr, unsigned numFrames) override;

private:
    std::unique_ptr<ResonantString[]> _strings;
    unsigned _numStrings = 0;
};

} // namespace sfz
} // namespace fx
