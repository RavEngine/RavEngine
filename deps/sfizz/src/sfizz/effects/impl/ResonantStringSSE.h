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

#pragma once
#include "SIMDConfig.h"

#if SFIZZ_CPU_FAMILY_X86_64 || SFIZZ_CPU_FAMILY_I386
#include "xmmintrin.h"

namespace sfz {
namespace fx {

class alignas(16) ResonantStringSSE {
public:
    void init(float sample_rate);
    void clear();
    void setGain(__m128 gain);
    void setResonanceFeedback(__m128 feedback);
    void setResonanceFrequency(__m128 frequency, __m128 bandwidth);
    __m128 process(__m128 input);

private:
    __m128 fConst0;
    __m128 fConst1;
    __m128 fRec0[2];
    __m128 fConst2;
    __m128 fConst3;
    __m128 fConst4;
    __m128 fConst5;
    __m128 fConst6;
    __m128 fConst7;
    __m128 fConst8;
    __m128 fRec2[3];
    __m128 fRec1[2];
    __m128 fControl[18];
};

} // namespace sfz
} // namespace fx
#endif
