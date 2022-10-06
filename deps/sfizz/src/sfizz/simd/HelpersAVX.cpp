// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "HelpersAVX.h"
#include "../SIMDConfig.h"
#include "../MathHelpers.h"
#include "Common.h"

#if SFIZZ_HAVE_AVX
#include <immintrin.h>
using Type = float;
constexpr unsigned TypeAlignment = 8;
constexpr unsigned ByteAlignment = TypeAlignment * sizeof(Type);
#endif

void gain1AVX(float gain, const float* input, float* output, unsigned size) noexcept
{
    const auto* sentinel = output + size;

#if SFIZZ_HAVE_AVX
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    const auto mmGain = _mm256_set1_ps(gain);
    while (unaligned<ByteAlignment>(input, output) && output < lastAligned)
        *output++ = gain * (*input++);

    while (output < lastAligned) {
        _mm256_store_ps(output, _mm256_mul_ps(mmGain, _mm256_load_ps(input)));
        incrementAll<TypeAlignment>(input, output);
    }
#endif

    while (output < sentinel)
        *output++ = gain * (*input++);
}

void gainAVX(const float* gain, const float* input, float* output, unsigned size) noexcept
{
    const auto* sentinel = output + size;

#if SFIZZ_HAVE_AVX
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(input, output) && output < lastAligned)
        *output++ = (*gain++) * (*input++);

    while (output < lastAligned) {
        _mm256_store_ps(output, _mm256_mul_ps(_mm256_load_ps(gain), _mm256_load_ps(input)));
        incrementAll<TypeAlignment>(input, output);
    }
#endif

    while (output < sentinel)
        *output++ = (*gain++) * (*input++);
}
