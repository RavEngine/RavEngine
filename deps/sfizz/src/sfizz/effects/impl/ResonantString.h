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

namespace sfz {
namespace fx {

class ResonantString {
public:
    void init(float sample_rate);
    void clear();
    void setGain(float gain);
    void setResonanceFeedback(float feedback);
    void setResonanceFrequency(float frequency, float bandwidth);
    float process(float input);

private:
    float fConst0;
    float fConst1;
    float fRec0[2];
    float fConst2;
    float fConst3;
    float fConst4;
    float fConst5;
    float fConst6;
    float fConst7;
    float fConst8;
    float fRec2[3];
    float fRec1[2];
    float fControl[18];
};

} // namespace sfz
} // namespace fx
