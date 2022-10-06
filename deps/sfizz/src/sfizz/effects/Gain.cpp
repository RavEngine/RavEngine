// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz


/**
   Note(jpc): this effect is book-only, mentioned but not documented

   Note(jpc): implementation status

- [x] gain
- [ ] gain_oncc
 */

#include "Gain.h"
#include "Opcode.h"
#include "SIMDHelpers.h"
#include "absl/memory/memory.h"

namespace sfz {
namespace fx {

    void Gain::setSampleRate(double sampleRate)
    {
        (void)sampleRate;
    }

    void Gain::setSamplesPerBlock(int samplesPerBlock)
    {
        _tempBuffer.resize(samplesPerBlock);
    }

    void Gain::clear()
    {
    }

    void Gain::process(const float* const inputs[], float* const outputs[], unsigned nframes)
    {
        const float baseGain = _gain;

        absl::Span<float> gains = _tempBuffer.getSpan(0);
        std::fill(gains.begin(), gains.end(), baseGain);

        // to linear
        for (unsigned i = 0; i < nframes; ++i)
            gains[i] = std::pow(10.0f, 0.05f * gains[i]);

        for (unsigned c = 0; c < EffectChannels; ++c) {
            absl::Span<const float> input { inputs[c], nframes };
            absl::Span<float> output { outputs[c], nframes };
            sfz::applyGain(absl::Span<const float> { gains }, input, output);
        }
    }

    std::unique_ptr<Effect> Gain::makeInstance(absl::Span<const Opcode> members)
    {
        Gain* gain = new Gain;
        std::unique_ptr<Effect> fx { gain };

        for (const Opcode& opc : members) {
            switch (opc.lettersOnlyHash) {
            case hash("gain"):
                gain->_gain = opc.read(Default::volume);
                break;
            }
        }

        return fx;
    }

} // namespace fx
} // namespace sfz
