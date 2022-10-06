// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Metronome.h"
#include "Config.h"

namespace sfz {

Metronome::Metronome()
{
    fGain = 0.5f;
    init(config::defaultSampleRate);
}

void Metronome::init(float sampleRate)
{
    fConst0 = std::min<float>(192000.0f, std::max<float>(1.0f, float(sampleRate)));
    fConst1 = std::cos((2764.60156f / fConst0));
    fConst2 = std::sqrt(std::max<float>(0.0f, ((fConst1 + 1.0f) / (1.0f - fConst1))));
    fConst3 = (1.0f / fConst2);
    fConst4 = std::cos((5529.20312f / fConst0));
    fConst5 = std::sqrt(std::max<float>(0.0f, ((fConst4 + 1.0f) / (1.0f - fConst4))));
    fConst6 = (1.0f / fConst5);
    fConst7 = std::max<float>(1.0f, (0.00499999989f * fConst0));
    fConst8 = (1.0f / fConst7);
    fConst9 = (1.0f / std::max<float>(1.0f, (0.100000001f * fConst0)));
    clear();
}

void Metronome::clear()
{
    for (int l0 = 0; (l0 < 2); l0 = (l0 + 1)) {
        iVec0[l0] = 0;
    }
    for (int l1 = 0; (l1 < 2); l1 = (l1 + 1)) {
        iVec1[l1] = 0;
    }
    for (int l2 = 0; (l2 < 2); l2 = (l2 + 1)) {
        iVec2[l2] = 0;
    }
    for (int l3 = 0; (l3 < 2); l3 = (l3 + 1)) {
        iRec0[l3] = 0;
    }
    for (int l4 = 0; (l4 < 2); l4 = (l4 + 1)) {
        fVec3[l4] = 0.0f;
    }
    for (int l5 = 0; (l5 < 2); l5 = (l5 + 1)) {
        fRec1[l5] = 0.0f;
    }
    for (int l6 = 0; (l6 < 2); l6 = (l6 + 1)) {
        fRec2[l6] = 0.0f;
    }
    for (int l7 = 0; (l7 < 2); l7 = (l7 + 1)) {
        fVec4[l7] = 0.0f;
    }
    for (int l8 = 0; (l8 < 2); l8 = (l8 + 1)) {
        fRec3[l8] = 0.0f;
    }
    for (int l9 = 0; (l9 < 2); l9 = (l9 + 1)) {
        fRec4[l9] = 0.0f;
    }
    for (int l10 = 0; (l10 < 2); l10 = (l10 + 1)) {
        iRec5[l10] = 0;
    }
}

void Metronome::processAdding(const int* beats, const int* beatsPerBar, float* outputL, float* outputR, int numFrames)
{
    float fSlow0 = float(fGain);
    for (int i = 0; (i < numFrames); i = (i + 1)) {
        int iTemp0 = int(beats[i]);
        iVec0[0] = iTemp0;
        iVec1[0] = 1;
        int iTemp1 = ((iTemp0 - iVec0[1]) > 0);
        iVec2[0] = iTemp1;
        iRec0[0] = (iTemp1 ? ((iTemp0 % int(beatsPerBar[i])) == 0) : iRec0[1]);
        fVec3[0] = fConst2;
        float fTemp2 = float((1 - iVec1[1]));
        float fTemp3 = (fConst3 * (fRec2[1] * (fTemp2 + fVec3[1])));
        float fTemp4 = (fConst1 * (fTemp3 + fRec1[1]));
        fRec1[0] = (fTemp4 + (fTemp2 + fTemp3));
        fRec2[0] = (fTemp4 - fRec1[1]);
        fVec4[0] = fConst5;
        float fTemp5 = (fConst6 * (fRec4[1] * (fTemp2 + fVec4[1])));
        float fTemp6 = (fConst4 * (fTemp5 + fRec3[1]));
        fRec3[0] = (fTemp6 + (fTemp2 + fTemp5));
        fRec4[0] = (fTemp6 - fRec3[1]);
        iRec5[0] = (((iRec5[1] + (iRec5[1] > 0)) * (iTemp1 <= iVec2[1])) + (iTemp1 > iVec2[1]));
        float fTemp7 = float(iRec5[0]);
        float fTemp8 = (fSlow0 * ((iRec0[0] ? (0.0f - (fConst5 * fRec4[0])) : (0.0f - (fConst2 * fRec2[0]))) * std::max<float>(0.0f, std::min<float>((fConst8 * fTemp7), ((fConst9 * (fConst7 - fTemp7)) + 1.0f)))));
        outputL[i] += float(fTemp8);
        outputR[i] += float(fTemp8);
        iVec0[1] = iVec0[0];
        iVec1[1] = iVec1[0];
        iVec2[1] = iVec2[0];
        iRec0[1] = iRec0[0];
        fVec3[1] = fVec3[0];
        fRec1[1] = fRec1[0];
        fRec2[1] = fRec2[0];
        fVec4[1] = fVec4[0];
        fRec3[1] = fRec3[0];
        fRec4[1] = fRec4[0];
        iRec5[1] = iRec5[0];
    }
}

} // namespace sfz
