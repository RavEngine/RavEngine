// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include <algorithm>
#include <cmath>

namespace sfz {

class Metronome {
public:
    Metronome();
    void init(float sampleRate);
    void clear();
    void processAdding(const int* beats, const int* beatsPerBar, float* outputL, float* outputR, int numFrames);
    void setGain(float gain) { fGain = gain; }

private:
    float fGain;
    int iVec0[2];
    int iVec1[2];
    int iVec2[2];
    int iRec0[2];
    float fConst0;
    float fConst1;
    float fConst2;
    float fVec3[2];
    float fConst3;
    float fRec1[2];
    float fRec2[2];
    float fConst4;
    float fConst5;
    float fVec4[2];
    float fConst6;
    float fRec3[2];
    float fRec4[2];
    float fConst7;
    float fConst8;
    int iRec5[2];
    float fConst9;

    /*
         import("stdfaust.lib");

         process(beats, beatsPerBar) = tone : *(envelope) <: (_, _) with {
           gain = hslider("[1] Gain", 0.5, 0.0, 1.0, 0.001);
           beatNumber = int(beats);
           beatIncrement = beatNumber-beatNumber';
           tone = (os.oscws(440.0), os.oscws(880.0)) : select2(toneSelect);
           toneSelect = x letrec { 'x = ba.if(beatIncrement>0, (beatNumber%int(beatsPerBar))==0, x); };
           envelope = (beatIncrement>0) : en.ar(5e-3, 100e-3) : *(gain);
         };
    */
};

} // namespace sfz
