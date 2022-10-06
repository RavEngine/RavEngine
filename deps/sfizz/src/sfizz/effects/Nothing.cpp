// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Nothing.h"
#include <cstring>

namespace sfz {
namespace fx {

    void Nothing::setSampleRate(double sampleRate)
    {
        (void)sampleRate;
    }

    void Nothing::setSamplesPerBlock(int samplesPerBlock)
    {
        (void)samplesPerBlock;
    }

    void Nothing::clear()
    {
    }

    void Nothing::process(const float* const inputs[], float* const outputs[], unsigned nframes)
    {
        for (unsigned c = 0; c < EffectChannels; ++c) {
            if (inputs[c] != outputs[c])
                std::memcpy(outputs[c], inputs[c], nframes * sizeof(float));
        }
    }

} // namespace fx
} // namespace sfz
