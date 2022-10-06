// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "ResonantArraySSE.h"
#include "Config.h"
#include <cstring>

#if SFIZZ_CPU_FAMILY_X86_64 || SFIZZ_CPU_FAMILY_I386
namespace sfz {
namespace fx {

static constexpr unsigned sseVectorSize = sizeof(__m128) / sizeof(float);

ResonantArraySSE::ResonantArraySSE()
{
    setSamplesPerBlock(config::defaultSamplesPerBlock);
}

ResonantArraySSE::~ResonantArraySSE()
{
}

void ResonantArraySSE::setup(
    float sampleRate, unsigned numStrings,
    const float pitches[], const float bandwidths[],
    const float feedbacks[], const float gains[])
{
    const unsigned numStringPacks = (numStrings + sseVectorSize - 1) / sseVectorSize;
    _stringPacks.resize(numStringPacks);
    ResonantStringSSE* stringPacks = _stringPacks.data();

    _numStrings = numStrings;

    for (unsigned p = 0; p < numStringPacks; ++p) {
        ResonantStringSSE& rs = stringPacks[p];
        rs.init(sampleRate);

        __m128 pitchSSE = _mm_set1_ps(0.0f);
        __m128 bandwidthSSE = _mm_set1_ps(0.0f);
        __m128 feedbackSSE = _mm_set1_ps(0.0f);
        __m128 gainSSE = _mm_set1_ps(0.0f);

        // copy 4 string parameters, or less if not enough remaining in buffer
        unsigned numCopy = std::min(sseVectorSize, numStrings - (p * sseVectorSize));

        std::memcpy(&pitchSSE, &pitches[p * sseVectorSize], numCopy * sizeof(float));
        std::memcpy(&bandwidthSSE, &bandwidths[p * sseVectorSize], numCopy * sizeof(float));
        std::memcpy(&feedbackSSE, &feedbacks[p * sseVectorSize], numCopy * sizeof(float));
        std::memcpy(&gainSSE, &gains[p * sseVectorSize], numCopy * sizeof(float));

        rs.setResonanceFrequency(pitchSSE, bandwidthSSE);
        rs.setResonanceFeedback(feedbackSSE);
        rs.setGain(gainSSE);
    }
}

void ResonantArraySSE::setSamplesPerBlock(unsigned samplesPerBlock)
{
    _workBuffer.resize(sseVectorSize * samplesPerBlock);
}

void ResonantArraySSE::clear()
{
    ResonantStringSSE* stringPacks = _stringPacks.data();
    const unsigned numStringPacks = (_numStrings + sseVectorSize - 1) / sseVectorSize;

    for (unsigned p = 0; p < numStringPacks; ++p) {
        ResonantStringSSE& rs = stringPacks[p];
        rs.clear();
    }
}

void ResonantArraySSE::process(const float *inPtr, float *outPtr, unsigned numFrames)
{
    ResonantStringSSE* stringPacks = _stringPacks.data();
    const unsigned numStringPacks = (_numStrings + sseVectorSize - 1) / sseVectorSize;

    // receive 4 resonator outputs per pack
    __m128* outputs4 = reinterpret_cast<__m128*>(_workBuffer.data());
    std::memset(outputs4, 0, numFrames * sizeof(__m128));

    for (unsigned p = 0; p < numStringPacks; ++p) {
        ResonantStringSSE& rs = stringPacks[p];
        for (unsigned i = 0; i < numFrames; ++i)
            outputs4[i] = _mm_add_ps(
                outputs4[i], rs.process(_mm_load1_ps(&inPtr[i])));
    }

    // sum resonator outputs 4 to 1
    for (unsigned i = 0; i < numFrames; ++i) {
        __m128 xmm0 = outputs4[i];
        __m128 xmm1 = _mm_shuffle_ps(xmm0, xmm0, 0xe5);
        __m128 xmm2 = _mm_movehl_ps(xmm0, xmm0);
        xmm1 = _mm_add_ss(xmm1, xmm0);
        xmm0 = _mm_shuffle_ps(xmm0, xmm0, 0xe7);
        xmm2 = _mm_add_ss(xmm2, xmm1);
        xmm0 = _mm_add_ss(xmm0, xmm2);
        outPtr[i] = _mm_cvtss_f32(xmm0);
    }
}

} // namespace sfz
} // namespace fx
#endif
