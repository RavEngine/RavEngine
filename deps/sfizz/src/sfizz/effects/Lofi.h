// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "Effects.h"
#include "OversamplerHelpers.h"

namespace sfz {
namespace fx {

    /**
     * @brief Bit crushing effect
     */
    class Lofi : public Effect {
    public:
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
        float _bitred_depth = 0;
        float _decim_depth = 0;

        ///
        class Bitred {
        public:
            void init(double sampleRate);
            void clear();
            void setDepth(float depth);
            void process(const float* in, float* out, uint32_t nframes);

        private:
            float fDepth = 0.0;
            float fLastValue = 0.0;
            hiir::Downsampler2x<12> fDownsampler2x;
        };

        ///
        class Decim {
        public:
            void init(double sampleRate);
            void clear();
            void setDepth(float depth);
            void process(const float* in, float* out, uint32_t nframes);

        private:
            float fSampleTime = 0.0;
            float fDepth = 0.0;
            float fPhase = 0.0;
            float fLastValue = 0.0;
            hiir::Downsampler2x<12> fDownsampler2x;
        };

        ///
        Bitred _bitred[EffectChannels];
        Decim _decim[EffectChannels];
    };

} // namespace fx
} // namespace sfz
