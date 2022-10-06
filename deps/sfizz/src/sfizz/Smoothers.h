// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once

#include "OnePoleFilter.h"
#include <array>

namespace sfz {
/**
 * @brief Wrapper class for a one pole filter smoother
 *
 */
class OnePoleSmoother {
public:
    OnePoleSmoother();
    /**
     * @brief Set the filter cutoff based on the sfz smoothing value
     * and the sample rate.
     *
     * @param smoothValue
     * @param sampleRate
     */
    void setSmoothing(unsigned smoothValue, float sampleRate);
    /**
     * @brief Reset the filter state to a given value
     *
     * @param value
     */
    void reset(float value = 0.0f);
    /**
     * @brief Reset to the target value (the back of the last vector passed)
     */
    void resetToTarget();
    /**
     * @brief Process a span of data. Input and output can refer to the same
     * memory.
     *
     * @param input
     * @param output
     * @param canShortcut whether we can have a fast path if the filter is within
     *                    a reasonable range around the first value of the input
     *                    span.
     */
    void process(absl::Span<const float> input, absl::Span<float> output, bool canShortcut = false);

    float current() const { return filter.current(); }
private:
    bool smoothing { false };
    OnePoleFilter<float> filter {};
    float target_ { 0.0f };
};

/**
 * @brief Linear smoother
 *
 */
class LinearSmoother {
public:
    LinearSmoother();
    /**
     * @brief Set the filter cutoff based on the sfz smoothing value
     * and the sample rate.
     *
     * @param smoothValue
     * @param sampleRate
     */
    void setSmoothing(unsigned smoothValue, float sampleRate);
    /**
     * @brief Reset the filter state to a given value
     *
     * @param value
     */
    void reset(float value = 0.0f);
    /**
     * @brief Reset to the target value (the back of the last vector passed)
     */
    void resetToTarget();
    /**
     * @brief Process a span of data. Input and output can refer to the same
     * memory.
     *
     * @param input
     * @param output
     * @param canShortcut whether we can have a fast path if the filter is within
     *                    a reasonable range around the first value of the input
     *                    span.
     */
    void process(absl::Span<const float> input, absl::Span<float> output, bool canShortcut = false);

    float current() const { return current_; }
private:
    float current_ = 0.0;
    float target_ = 0.0;
    float step_ = 0.0;
    //int32_t framesToTarget_ = 0;
    int32_t smoothFrames_ = 0;
};

/**
 * @brief Default smoother
 */
using Smoother = LinearSmoother;

}
