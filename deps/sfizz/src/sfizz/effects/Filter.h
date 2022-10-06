// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "Effects.h"
#include "FilterDescription.h"
#include "SfzFilter.h"

namespace sfz {
namespace fx {

    /**
     * @brief Effect which passes signal through a filter
     */
    class Filter : public Effect {
    public:
        explicit Filter(const FilterDescription& desc);

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
        void prepareFilter();

    private:
        sfz::Filter _filter;
        FilterDescription _desc;
        AudioBuffer<float, 3> _tempBuffer { 3, config::defaultSamplesPerBlock };
    };

} // namespace fx
} // namespace sfz
