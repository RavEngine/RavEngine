// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "Effects.h"
#include "AudioBuffer.h"
#include <memory>

namespace sfz {
namespace fx {

    class ResonantArray;

    /**
     * @brief String resonance effect
     */
    class Strings : public Effect {
    public:
        Strings();
        ~Strings();

        /**
         * @brief Initializes with the given sample rate.
         */
        void setSampleRate(double sampleRate) override;

        /**
         * @brief Sets the maximum number of frames to render at a time. The actual
         * value can be lower but should never be higher.
         */
        void setSamplesPerBlock(int samplesPerBlock) override;

        /**
         * @brief Reset the state to initial.
         */
        void clear() override;

        /**
         * @brief Copy the input signal to the output
         */
        void process(const float* const inputs[], float* const outputs[], unsigned nframes) override;

        /**
          * @brief Instantiates given the contents of the <effect> block.
          */
        static std::unique_ptr<Effect> makeInstance(absl::Span<const Opcode> members);

    private:
        enum { MaximumNumStrings = 88 };

        unsigned _numStrings { Default::maxStrings };
        float _wet { Default::effect };

        std::unique_ptr<ResonantArray> _stringsArray;

        AudioBuffer<float, 3> _tempBuffer { 3, config::defaultSamplesPerBlock };
    };

} // namespace fx
} // namespace sfz
