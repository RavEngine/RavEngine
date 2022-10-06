// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "ResonantArrayAVX.h"
#include "Config.h"
#include <cstring>

#if SFIZZ_CPU_FAMILY_X86_64 || SFIZZ_CPU_FAMILY_I386
namespace sfz {
namespace fx {

static constexpr unsigned avxVectorSize = sizeof(__m256) / sizeof(float);

ResonantArrayAVX::ResonantArrayAVX()
{
    setSamplesPerBlock(config::defaultSamplesPerBlock);
}

ResonantArrayAVX::~ResonantArrayAVX()
{
}

void ResonantArrayAVX::setup(
    float sampleRate, unsigned numStrings,
    const float pitches[], const float bandwidths[],
    const float feedbacks[], const float gains[])
{
    const unsigned numStringPacks = (numStrings + avxVectorSize - 1) / avxVectorSize;
    _stringPacks.resize(numStringPacks);
    ResonantStringAVX* stringPacks = _stringPacks.data();

    _numStrings = numStrings;

    for (unsigned p = 0; p < numStringPacks; ++p) {
        ResonantStringAVX& rs = stringPacks[p];
        rs.init(sampleRate);

        __m256 pitchAVX = _mm256_set1_ps(0.0f);
        __m256 bandwidthAVX = _mm256_set1_ps(0.0f);
        __m256 feedbackAVX = _mm256_set1_ps(0.0f);
        __m256 gainAVX = _mm256_set1_ps(0.0f);

        // copy 8 string parameters, or less if not enough remaining in buffer
        unsigned numCopy = std::min(avxVectorSize, numStrings - (p * avxVectorSize));

        std::memcpy(&pitchAVX, &pitches[p * avxVectorSize], numCopy * sizeof(float));
        std::memcpy(&bandwidthAVX, &bandwidths[p * avxVectorSize], numCopy * sizeof(float));
        std::memcpy(&feedbackAVX, &feedbacks[p * avxVectorSize], numCopy * sizeof(float));
        std::memcpy(&gainAVX, &gains[p * avxVectorSize], numCopy * sizeof(float));

        rs.setResonanceFrequency(pitchAVX, bandwidthAVX);
        rs.setResonanceFeedback(feedbackAVX);
        rs.setGain(gainAVX);
    }
}

void ResonantArrayAVX::setSamplesPerBlock(unsigned samplesPerBlock)
{
    _workBuffer.resize(avxVectorSize * samplesPerBlock);
}

void ResonantArrayAVX::clear()
{
    ResonantStringAVX* stringPacks = _stringPacks.data();
    const unsigned numStringPacks = (_numStrings + avxVectorSize - 1) / avxVectorSize;

    for (unsigned p = 0; p < numStringPacks; ++p) {
        ResonantStringAVX& rs = reinterpret_cast<ResonantStringAVX&>(stringPacks[p]);
        rs.clear();
    }
}

void ResonantArrayAVX::process(const float *inPtr, float *outPtr, unsigned numFrames)
{
    ResonantStringAVX* stringPacks = _stringPacks.data();
    const unsigned numStringPacks = (_numStrings + avxVectorSize - 1) / avxVectorSize;

    // receive 8 resonator outputs per pack
    __m256* outputs8 = reinterpret_cast<__m256*>(_workBuffer.data());
    std::memset(outputs8, 0, numFrames * sizeof(__m256));

    for (unsigned p = 0; p < numStringPacks; ++p) {
        ResonantStringAVX& rs = reinterpret_cast<ResonantStringAVX&>(stringPacks[p]);
        for (unsigned i = 0; i < numFrames; ++i)
            outputs8[i] = _mm256_add_ps(
                outputs8[i], rs.process(_mm256_broadcast_ss(&inPtr[i])));
    }

    // sum resonator outputs 8 to 1
    for (unsigned i = 0; i < numFrames; ++i) {
        __m256 x = outputs8[i];
        const __m128 x128 = _mm_add_ps(_mm256_extractf128_ps(x, 1), _mm256_castps256_ps128(x));
        const __m128 x64 = _mm_add_ps(x128, _mm_movehl_ps(x128, x128));
        const __m128 x32 = _mm_add_ss(x64, _mm_shuffle_ps(x64, x64, 0x55));
        outPtr[i] = _mm_cvtss_f32(x32);
    }
}

} // namespace sfz
} // namespace fx
#endif
