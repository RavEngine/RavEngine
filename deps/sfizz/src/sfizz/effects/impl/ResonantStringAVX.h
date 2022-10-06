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
#include "immintrin.h"

namespace sfz {
namespace fx {

class alignas(32) ResonantStringAVX {
public:
    void init(float sample_rate);
    void clear();
    void setGain(__m256 gain);
    void setResonanceFeedback(__m256 feedback);
    void setResonanceFrequency(__m256 frequency, __m256 bandwidth);
    __m256 process(__m256 input);

private:
    __m256 fConst0;
    __m256 fConst1;
    __m256 fRec0[2];
    __m256 fConst2;
    __m256 fConst3;
    __m256 fConst4;
    __m256 fConst5;
    __m256 fConst6;
    __m256 fConst7;
    __m256 fConst8;
    __m256 fRec2[3];
    __m256 fRec1[2];
    __m256 fControl[18];
};

} // namespace sfz
} // namespace fx
#endif
