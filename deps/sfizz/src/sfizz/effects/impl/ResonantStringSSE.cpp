// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

/**
   Note(jpc): generated with faust and edited
 */

/* ------------------------------------------------------------
name: "resonant_string"
Code generated with Faust 2.20.2 (https://faust.grame.fr)
Compilation options: -lang cpp -inpl -os -scal -ftz 0
------------------------------------------------------------ */

#include "ResonantStringSSE.h"

#if SFIZZ_CPU_FAMILY_X86_64 || SFIZZ_CPU_FAMILY_I386
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <cstdint>

namespace sfz {
namespace fx {

static float faustpower2_f(float value)
{
    return (value * value);
}

static __m128 faustpower2_v(__m128 value)
{
    return _mm_mul_ps(value, value);
}

static float load_nth_v(const __m128 &x, unsigned i)
{
    return reinterpret_cast<const float *>(&x)[i];
}

static void store_nth_v(__m128 &x, unsigned i, float v)
{
    reinterpret_cast<float *>(&x)[i] = v;
}

void ResonantStringSSE::init(float sample_rate)
{
    if (reinterpret_cast<uintptr_t>(this) & 15)
        throw std::runtime_error("The resonant string is misaligned for SSE");

    fConst0 = _mm_set1_ps(sample_rate);
    fConst1 = _mm_div_ps(_mm_set1_ps(6.28318548f), fConst0);
    fConst2 = _mm_div_ps(_mm_set1_ps(2.0f), fConst0);
    fConst3 = _mm_mul_ps(_mm_set1_ps(2.0f), fConst0);
    fConst4 = _mm_div_ps(_mm_set1_ps(3.14159274f), fConst0);
    fConst5 = _mm_div_ps(_mm_set1_ps(0.5f), fConst0);
    fConst6 = _mm_mul_ps(_mm_set1_ps(4.0f), faustpower2_v(fConst0));
    fConst7 = faustpower2_v(_mm_div_ps(_mm_set1_ps(1.0f), fConst0));
    fConst8 = _mm_mul_ps(_mm_set1_ps(2.0f), fConst7);

    clear();
}

void ResonantStringSSE::clear()
{
    for (int l0 = 0; (l0 < 2); l0 = (l0 + 1)) {
        fRec0[l0] = _mm_set1_ps(0.0f);
    }
    for (int l1 = 0; (l1 < 3); l1 = (l1 + 1)) {
        fRec2[l1] = _mm_set1_ps(0.0f);
    }
    for (int l2 = 0; (l2 < 2); l2 = (l2 + 1)) {
        fRec1[l2] = _mm_set1_ps(0.0f);
    }
}

void ResonantStringSSE::setGain(__m128 gain)
{
    fControl[0] = gain;
}

void ResonantStringSSE::setResonanceFeedback(__m128 feedback)
{
    fControl[1] = feedback;
}

void ResonantStringSSE::setResonanceFrequency(__m128 frequency, __m128 bandwidth)
{
    fControl[2] = frequency;
    fControl[3] = _mm_mul_ps(fConst1, fControl[2]);
    for (int i = 0; i < int(sizeof(__m128) / sizeof(float)); ++i) {
        store_nth_v(fControl[4], i, std::sin(load_nth_v(fControl[3], i)));
        store_nth_v(fControl[5], i, std::cos(load_nth_v(fControl[3], i)));
    }
    fControl[6] = _mm_mul_ps(_mm_set1_ps(0.5f), bandwidth);
    for (int i = 0; i < int(sizeof(__m128) / sizeof(float)); ++i) {
        store_nth_v(fControl[7], i, std::tan((load_nth_v(fConst4, i) * (load_nth_v(fControl[6], i) + load_nth_v(fControl[2], i)))));
        store_nth_v(fControl[8], i, faustpower2_f(std::sqrt((load_nth_v(fConst6, i) * (load_nth_v(fControl[7], i) * std::tan((load_nth_v(fConst4, i) * (load_nth_v(fControl[2], i) - load_nth_v(fControl[6], i)))))))));
    }
    fControl[9] = _mm_sub_ps(_mm_mul_ps(fConst3, fControl[7]), _mm_mul_ps(fConst5, _mm_div_ps(fControl[8], fControl[7])));
    fControl[10] = _mm_mul_ps(fConst7, fControl[8]);
    fControl[11] = _mm_mul_ps(fConst2, fControl[9]);
    fControl[12] = _mm_add_ps(_mm_add_ps(fControl[10], fControl[11]), _mm_set1_ps(4.0f));
    fControl[13] = _mm_mul_ps(fConst2, _mm_div_ps(fControl[9], fControl[12]));
    fControl[14] = _mm_sub_ps(_mm_set1_ps(0.0f), fControl[13]);
    fControl[15] = _mm_div_ps(_mm_set1_ps(1.0f), fControl[12]);
    fControl[16] = _mm_add_ps(_mm_mul_ps(fConst8, fControl[8]), _mm_set1_ps(-8.0f));
    fControl[17] = _mm_add_ps(fControl[10], _mm_sub_ps(_mm_set1_ps(4.0f), fControl[11]));
}

__m128 ResonantStringSSE::process(__m128 input)
{
    fRec0[0] = _mm_mul_ps(fControl[1], _mm_add_ps(_mm_mul_ps(fControl[4], fRec1[1]), _mm_mul_ps(fControl[5], fRec0[1])));
    __m128 fTemp0 = input;
    fRec2[0] = _mm_sub_ps(fTemp0, _mm_mul_ps(fControl[15], _mm_add_ps(_mm_mul_ps(fControl[16], fRec2[1]), _mm_mul_ps(fControl[17], fRec2[2]))));
    fRec1[0] = _mm_sub_ps(_mm_add_ps(_mm_mul_ps(fControl[14], fRec2[2]), _mm_add_ps(_mm_mul_ps(fControl[5], fRec1[1]), _mm_mul_ps(fControl[13], fRec2[0]))),_mm_mul_ps(fControl[4], fRec0[1]));
    __m128 output = _mm_mul_ps(fControl[0], fRec0[0]);
    fRec0[1] = fRec0[0];
    fRec2[2] = fRec2[1];
    fRec2[1] = fRec2[0];
    fRec1[1] = fRec1[0];
    return output;
}

} // namespace sfz
} // namespace fx
#endif
