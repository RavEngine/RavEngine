// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "PowerFollower.h"
#include "Defaults.h"
#include "SIMDHelpers.h"
#include <absl/types/span.h>

namespace sfz {

PowerFollower::PowerFollower()
    : sampleRate_(config::defaultSampleRate),
      samplesPerBlock_(config::defaultSamplesPerBlock),
      tempBuffer_(new float[config::defaultSamplesPerBlock])
{
    updateTrackingFactor();
}

void PowerFollower::setSampleRate(float sampleRate) noexcept
{
    if (sampleRate_ != sampleRate) {
        sampleRate_ = sampleRate;
        updateTrackingFactor();
    }
}

void PowerFollower::setSamplesPerBlock(unsigned samplesPerBlock)
{
    if (samplesPerBlock_ != samplesPerBlock) {
        tempBuffer_.reset(new float[samplesPerBlock]);
        samplesPerBlock_ = samplesPerBlock;
    }
}

void PowerFollower::process(AudioSpan<float> buffer) noexcept
{
    size_t numFrames = buffer.getNumFrames();
    if (numFrames == 0)
        return;

    ///
    constexpr size_t step = config::powerFollowerStep;
    float currentPower = currentPower_;
    float currentSum = currentSum_;
    size_t currentCount = currentCount_;

    const float attackFactor = attackTrackingFactor_;
    const float releaseFactor = releaseTrackingFactor_;

    ///
    size_t index = 0;
    while (index < numFrames) {
        size_t blockSize = std::min(step - currentCount, numFrames - index);
        absl::Span<float> tempBuffer(tempBuffer_.get(), blockSize);

        copy(buffer.getConstSpan(0).subspan(index, blockSize), tempBuffer);
        for (unsigned i = 1, n = buffer.getNumChannels(); i < n; ++i)
            add(buffer.getConstSpan(i).subspan(index, blockSize), tempBuffer);

        currentSum += sumSquares<float>(tempBuffer);
        currentCount += blockSize;

        if (currentCount == step) {
            const float meanPower = currentSum / step;
            currentPower = max(
                currentPower * attackFactor + meanPower * (1 - attackFactor),
                currentPower * releaseFactor + meanPower * (1 - releaseFactor));
            currentSum = 0;
            currentCount = 0;
        }

        index += blockSize;
    }

    ///
    currentPower_ = currentPower;
    currentSum_ = currentSum;
    currentCount_ = currentCount;
}

void PowerFollower::clear() noexcept
{
    currentPower_ = 0;
    currentSum_ = 0;
    currentCount_ = 0;
}

void PowerFollower::updateTrackingFactor() noexcept
{
    // Protect the envelope follower against blowups
    attackTrackingFactor_ = std::exp(-1.0f / ((config::powerFollowerAttackTime / config::powerFollowerStep) * sampleRate_));
    releaseTrackingFactor_ = std::exp(-1.0f / ((config::powerFollowerReleaseTime / config::powerFollowerStep) * sampleRate_));
}

} // namespace sfz
