// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "utility/NumericId.h"
#include <absl/types/span.h>
#include <cstdint>

namespace sfz {

class ModKey;
class Voice;

/**
 * @brief Generator for modulation sources
 */
class ModGenerator {
public:
    virtual ~ModGenerator() {}

    /**
     * @brief Set the sample rate
     */
    virtual void setSampleRate(double sampleRate) { (void)sampleRate; }

    /**
     * @brief Set the maximum block size
     */
    virtual void setSamplesPerBlock(unsigned count) { (void)count; }

    /**
     * @brief Initialize the generator.
     *
     * @param sourceKey identifier of the source to initialize
     * @param voiceId the particular voice to initialize, if per-voice
     * @param delay the frame time when it happens
     */
    virtual void init(const ModKey& sourceKey, NumericId<Voice> voiceId, unsigned delay) = 0;

    /**
     * @brief Send the generator a release notification.
     *
     * @param sourceKey identifier of the source to release
     * @param voiceId the particular voice to initialize, if per-voice
     * @param delay the frame time when it happens
     */
    virtual void release(const ModKey& sourceKey, NumericId<Voice> voiceId, unsigned delay) { (void)sourceKey; (void)voiceId; (void)delay; }

    /**
     * @brief Cancel the release and get back into sustain
     *
     * @param sourceKey identifier of the source to release
     * @param voiceId the particular voice to initialize, if per-voice
     * @param delay the frame time when it happens
     */
    virtual void cancelRelease(const ModKey& sourceKey, NumericId<Voice> voiceId, unsigned delay) { (void)sourceKey; (void)voiceId; (void)delay; }

    /**
     * @brief Generate a cycle of the modulator
     *
     * @param sourceKey source key
     * @param voiceNum voice number if the generator is per-voice, otherwise undefined
     * @param buffer output buffer
     */
    virtual void generate(const ModKey& sourceKey, NumericId<Voice> voiceNum, absl::Span<float> buffer) = 0;

    /**
     * @brief Advance the generator by a number of frames
     * This is called instead of `generate` in case the output is discarded.
     * It can be overriden with a faster implementation if wanted.
     *
     * @param sourceKey source key
     * @param voiceNum voice number if the generator is per-voice, otherwise undefined
     * @param buffer writable spare buffer, contents will be discarded
     */
    virtual void generateDiscarded(const ModKey& sourceKey, NumericId<Voice> voiceNum, absl::Span<float> buffer)
    {
        generate(sourceKey, voiceNum, buffer);
    }
};

} // namespace sfz
