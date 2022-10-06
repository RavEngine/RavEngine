// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz


/**
   Note(jpc): this effect is book-only, mentioned but not documented

   Note(jpc): implementation status

- [x] width
- [ ] width_oncc
 */

#include "Width.h"
#include "Panning.h"
#include "Opcode.h"
#include "absl/memory/memory.h"

namespace sfz {
namespace fx {

    void Width::setSampleRate(double sampleRate)
    {
        (void)sampleRate;
    }

    void Width::setSamplesPerBlock(int samplesPerBlock)
    {
        _tempBuffer.resize(samplesPerBlock);
    }

    void Width::clear()
    {
    }

    void Width::process(const float* const inputs[], float* const outputs[], unsigned nframes)
    {
        const float baseWidth = _width;

        absl::Span<float> widths = _tempBuffer.getSpan(0);
        std::fill(widths.begin(), widths.end(), baseWidth);

        absl::Span<const float> input1 { inputs[0], nframes };
        absl::Span<const float> input2 { inputs[1], nframes };
        absl::Span<float> output1 { outputs[0], nframes };
        absl::Span<float> output2 { outputs[1], nframes };

        for (unsigned i = 0; i < nframes; ++i) {
            const float l = input1[i];
            const float r = input2[i];

            const float w = clamp((widths[i] + 100.0f) * 0.005f, 0.0f, 1.0f);
            const float coeff1 = panLookup(w);
            const float coeff2 = panLookup(1.0f - w);

            output1[i] = l * coeff2 + r * coeff1;
            output2[i] = l * coeff1 + r * coeff2;
        }
    }

    std::unique_ptr<Effect> Width::makeInstance(absl::Span<const Opcode> members)
    {
        Width* width = new Width;
        std::unique_ptr<Effect> fx { width };

        for (const Opcode& opc : members) {
            switch (opc.lettersOnlyHash) {
            case hash("width"):
                width->_width = opc.read(Default::width);
                break;
            }
        }

        return fx;
    }

} // namespace fx
} // namespace sfz
