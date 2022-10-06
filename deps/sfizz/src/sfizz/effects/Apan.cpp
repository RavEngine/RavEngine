// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

/*
   Note(jpc): implementation status

- [x] apan_waveform
- [x] apan_freq
- [ ] apan_freq_oncc
- [x] apan_phase
- [ ] apan_phase_oncc
- [x] apan_dry
- [ ] apan_dry_oncc
- [x] apan_wet
- [ ] apan_wet_oncc
- [x] apan_depth
- [ ] apan_depth_oncc
*/

#include "Apan.h"
#include "LFOCommon.h"
#include "Opcode.h"
#include "utility/Macros.h"
#include <limits>
#include <cmath>

namespace sfz {
namespace fx {

    void Apan::setSampleRate(double sampleRate)
    {
        _samplePeriod = 1.0 / sampleRate;
    }

    void Apan::setSamplesPerBlock(int samplesPerBlock)
    {
        _lfoOutLeft.resize(samplesPerBlock);
        _lfoOutRight.resize(samplesPerBlock);
    }

    void Apan::clear()
    {
        _lfoPhase = 0.0;
    }

    void Apan::process(const float* const inputs[], float* const outputs[], unsigned nframes)
    {
        float dry = _dry;
        float wet = _wet;
        float depth = _depth;
        float* modL = _lfoOutLeft.data();
        float* modR = _lfoOutRight.data();

        computeLfos(modL, modR, nframes);

        const float* inL = inputs[0];
        const float* inR = inputs[1];
        float* outL = outputs[0];
        float* outR = outputs[1];

        for (unsigned i = 0; i < nframes; ++i) {
            float modDD = depth * 0.5f * (modL[i] - modR[i]); // LFO in ±depth

            // uses a linear pan law
            float gainL = 1 - modDD;
            float gainR = 1 + modDD;

            outL[i] = inL[i] * (gainL * wet + dry);
            outR[i] = inR[i] * (gainR * wet + dry);
        }
    }

    std::unique_ptr<Effect> Apan::makeInstance(absl::Span<const Opcode> members)
    {
        Apan* apan = new Apan;
        std::unique_ptr<Effect> fx { apan };

        for (const Opcode& opc : members) {
            switch (opc.lettersOnlyHash) {
            case hash("apan_waveform"):
                apan->_lfoWave = opc.read(Default::apanWaveform);
                break;
            case hash("apan_freq"):
                apan->_lfoFrequency = opc.read(Default::apanFrequency);
                break;
            case hash("apan_phase"):
                apan->_lfoPhaseOffset = opc.read(Default::apanPhase);
                break;
            case hash("apan_dry"):
                apan->_dry = opc.read(Default::apanLevel);
                break;
            case hash("apan_wet"):
                apan->_wet = opc.read(Default::apanLevel);
                break;
            case hash("apan_depth"):
                apan->_depth = opc.read(Default::apanLevel);
                break;
            }
        }

        return fx;
    }

    void Apan::computeLfos(float* left, float* right, unsigned nframes)
    {
        switch (_lfoWave) {
        #define CASE(X) case X:                          \
            computeLfos<X>(left, right, nframes); break;
        default:
        CASE(LFOWave::Triangle)
        CASE(LFOWave::Sine)
        CASE(LFOWave::Pulse75)
        CASE(LFOWave::Square)
        CASE(LFOWave::Pulse25)
        CASE(LFOWave::Pulse12_5)
        CASE(LFOWave::Ramp)
        CASE(LFOWave::Saw)
        #undef CASE
        }
    }

    template <LFOWave Wave> void Apan::computeLfos(float* left, float* right, unsigned nframes)
    {
        float samplePeriod = _samplePeriod;
        float frequency = _lfoFrequency;
        float offset = _lfoPhaseOffset;
        float phaseLeft = _lfoPhase;

        for (unsigned i = 0; i < nframes; ++i) {
            float phaseRight = phaseLeft + offset;
            phaseRight -= static_cast<int>(phaseRight);

            left[i] = lfo::evaluateAtPhase<Wave>(phaseLeft);
            right[i] = lfo::evaluateAtPhase<Wave>(phaseRight);

            phaseLeft += frequency * samplePeriod;
            phaseLeft -= static_cast<int>(phaseLeft);
        }

        _lfoPhase = phaseLeft;
    }

} // namespace fx
} // namespace sfz
