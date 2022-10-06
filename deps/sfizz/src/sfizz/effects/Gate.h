// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "Effects.h"
#include <memory>

namespace sfz {
namespace fx {

    /**
     * @brief Gate effect
     */
    class Gate : public Effect {
    public:
        Gate();
        ~Gate();

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
        struct Impl;
        std::unique_ptr<Impl> _impl;
    };

} // namespace fx
} // namespace sfz
