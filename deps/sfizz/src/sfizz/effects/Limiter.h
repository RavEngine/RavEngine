// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "Effects.h"
#include "OversamplerHelpers.h"
class faustLimiter;

namespace sfz {
namespace fx {

    /**
     * @brief Limiter effect
     */
    class Limiter : public Effect {
    public:
        Limiter();
        ~Limiter();

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
          * @brief Computes a cycle of the effect in stereo.
          */
        void process(const float* const inputs[], float* const outputs[], unsigned nframes) override;

        /**
          * @brief Instantiates given the contents of the <effect> block.
          */
        static std::unique_ptr<Effect> makeInstance(absl::Span<const Opcode> members);

    private:
        std::unique_ptr<faustLimiter> _limiter;
        AudioBuffer<float, 2> _tempBuffer2x { 2, 2 * config::defaultSamplesPerBlock };
        hiir::Downsampler2x<12> _downsampler2x[EffectChannels];
        hiir::Upsampler2x<12> _upsampler2x[EffectChannels];
    };

} // namespace fx
} // namespace sfz
