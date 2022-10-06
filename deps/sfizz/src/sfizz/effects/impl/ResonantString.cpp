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

#include "ResonantString.h"
#include <algorithm>
#include <cmath>
#include <math.h>

namespace sfz {
namespace fx {

static float faustpower2_f(float value)
{
    return (value * value);
}

void ResonantString::init(float sample_rate)
{
    fConst0 = sample_rate;
    fConst1 = (6.28318548f / fConst0);
    fConst2 = (2.0f / fConst0);
    fConst3 = (2.0f * fConst0);
    fConst4 = (3.14159274f / fConst0);
    fConst5 = (0.5f / fConst0);
    fConst6 = (4.0f * faustpower2_f(fConst0));
    fConst7 = faustpower2_f((1.0f / fConst0));
    fConst8 = (2.0f * fConst7);

    clear();
}

void ResonantString::clear()
{
    for (int l0 = 0; (l0 < 2); l0 = (l0 + 1)) {
        fRec0[l0] = 0.0f;
    }
    for (int l1 = 0; (l1 < 3); l1 = (l1 + 1)) {
        fRec2[l1] = 0.0f;
    }
    for (int l2 = 0; (l2 < 2); l2 = (l2 + 1)) {
        fRec1[l2] = 0.0f;
    }
}

void ResonantString::setGain(float gain)
{
    fControl[0] = gain;
}

void ResonantString::setResonanceFeedback(float feedback)
{
    fControl[1] = feedback;
}

void ResonantString::setResonanceFrequency(float frequency, float bandwidth)
{
    fControl[2] = frequency;
    fControl[3] = (fConst1 * fControl[2]);
    fControl[4] = std::sin(fControl[3]);
    fControl[5] = std::cos(fControl[3]);
    fControl[6] = (0.5f * bandwidth);
    fControl[7] = std::tan((fConst4 * (fControl[6] + fControl[2])));
    fControl[8] = faustpower2_f(std::sqrt((fConst6 * (fControl[7] * std::tan((fConst4 * (fControl[2] - fControl[6])))))));
    fControl[9] = ((fConst3 * fControl[7]) - (fConst5 * (fControl[8] / fControl[7])));
    fControl[10] = (fConst7 * fControl[8]);
    fControl[11] = (fConst2 * fControl[9]);
    fControl[12] = ((fControl[10] + fControl[11]) + 4.0f);
    fControl[13] = (fConst2 * (fControl[9] / fControl[12]));
    fControl[14] = (0.0f - fControl[13]);
    fControl[15] = (1.0f / fControl[12]);
    fControl[16] = ((fConst8 * fControl[8]) + -8.0f);
    fControl[17] = (fControl[10] + (4.0f - fControl[11]));
}

float ResonantString::process(float input)
{
    fRec0[0] = (fControl[1] * ((fControl[4] * fRec1[1]) + (fControl[5] * fRec0[1])));
    float fTemp0 = input;
    fRec2[0] = (fTemp0 - (fControl[15] * ((fControl[16] * fRec2[1]) + (fControl[17] * fRec2[2]))));
    fRec1[0] = (((fControl[14] * fRec2[2]) + ((fControl[5] * fRec1[1]) + (fControl[13] * fRec2[0]))) - (fControl[4] * fRec0[1]));
    float output = float((fControl[0] * fRec0[0]));
    fRec0[1] = fRec0[0];
    fRec2[2] = fRec2[1];
    fRec2[1] = fRec2[0];
    fRec1[1] = fRec1[0];
    return output;
}

} // namespace sfz
} // namespace fx
