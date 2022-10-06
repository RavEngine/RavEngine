// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "Buffer.h"
#include "AudioBuffer.h"
#include "AudioSpan.h"
#include "Config.h"
#include "utility/Debug.h"
#include <absl/types/span.h>
#include <array>
#include <memory>

namespace sfz {
class AudioReader;

enum class Oversampling: int {
    x1 = 1,
    x2 = 2,
    x4 = 4,
    x8 = 8
};

/**
 * @brief Wraps the internal oversampler in a single function that takes an
 *        AudioBuffer and oversamples it in another pre-allocated one. The
 *        Oversampler processes the file in chunks and can signal the frames
 *        processes using an atomic counter.
 */
class Oversampler
{
public:
    /**
     * @brief Construct a new Oversampler object
     *
     * @param factor
     * @param chunkSize
     */
    Oversampler(Oversampling factor = Oversampling::x1, size_t chunkSize = config::fileChunkSize);
    /**
     * @brief Stream the oversampling of an input AudioBuffer into an output
     *        one, possibly signaling the caller along the way of the number of
     *        frames that are written.
     *
     * @param input
     * @param output
     * @param framesReady an atomic counter for the ready frames. If null no signaling is done.
     */
    void stream(AudioSpan<float> input, AudioSpan<float> output, std::atomic<size_t>* framesReady = nullptr);
    /**
     * @brief Stream the oversampling of an input AudioReader into an output
     *        one, possibly signaling the caller along the way of the number of
     *        frames that are written.
     *
     * @param input
     * @param output
     * @param framesReady an atomic counter for the ready frames. If null no signaling is done.
     */
    void stream(AudioReader& input, AudioSpan<float> output, std::atomic<size_t>* framesReady = nullptr);

    Oversampler() = delete;
    Oversampler(const Oversampler&) = delete;
    Oversampler(Oversampler&&) = delete;
private:
    Oversampling factor;
    size_t chunkSize;

    LEAK_DETECTOR(Oversampler);
};

}
