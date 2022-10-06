// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

/*
   Note(jpc): implementation status

- [ ] egN_points (purpose unknown)
- [x] egN_timeX
- [x] egN_timeX_onccY
- [x] egN_levelX
- [x] egN_levelX_onccY
- [x] egN_shapeX
- [ ] egN_shapeX_onccY
- [x] egN_sustain
- [ ] egN_dynamic
- [ ] egN_loop
- [ ] egN_loop_shape
- [ ] egN_loop_count
*/

#include "FlexEnvelope.h"
#include "FlexEGDescription.h"
#include "Curve.h"
#include "MidiState.h"
#include "Resources.h"
#include "Config.h"
#include "SIMDHelpers.h"
#include <absl/types/optional.h>

namespace sfz {

struct FlexEnvelope::Impl {
    const Resources* resources_ { nullptr };
    const FlexEGDescription* desc_ { nullptr };
    float samplePeriod_ { 1.0 / config::defaultSampleRate };
    size_t delayFramesLeft_ { 0 };

    //
    float stageSourceLevel_ { 0.0 };
    float stageTargetLevel_ { 0.0 };
    float stageTime_ { 0.0 };
    bool stageSustained_ { false };
    const Curve* stageCurve_ { nullptr };

    //
    unsigned currentStageNumber_ { 0 };
    float currentLevel_ { 0.0 };
    float currentTime_ { 0.0 };
    absl::optional<size_t> currentFramesUntilRelease_ { absl::nullopt };
    bool isReleased_ { false };
    bool freeRunning_ { false };

    //
    void process(absl::Span<float> out);
    bool advanceToStage(unsigned stageNumber);
    bool advanceToNextStage();
    void updateCurrentTimeAndLevel(int delay = 0);
};

FlexEnvelope::FlexEnvelope(Resources &resources)
    : impl_(new Impl)
{
    Impl& impl = *impl_;
    impl.resources_ = &resources;
}

FlexEnvelope::~FlexEnvelope()
{
}

void FlexEnvelope::setSampleRate(double sampleRate)
{
    Impl& impl = *impl_;
    impl.samplePeriod_ = 1.0 / sampleRate;
}

void FlexEnvelope::configure(const FlexEGDescription* desc)
{
    Impl& impl = *impl_;
    impl.desc_ = desc;

    //
    impl.freeRunning_ = false;
    impl.isReleased_ = false;

    //
    impl.currentStageNumber_ = 0;
    impl.currentLevel_ = 0.0;
    impl.currentTime_ = 0.0;
}

void FlexEnvelope::start(unsigned triggerDelay)
{
    Impl& impl = *impl_;
    impl.delayFramesLeft_ = triggerDelay;
    impl.currentFramesUntilRelease_ = absl::nullopt;
    impl.advanceToStage(0);
}

void FlexEnvelope::setFreeRunning(bool freeRunning)
{
    Impl& impl = *impl_;
    impl.freeRunning_ = freeRunning;
}

void FlexEnvelope::release(unsigned releaseDelay)
{
    Impl& impl = *impl_;
    impl.currentFramesUntilRelease_ = releaseDelay;
}

void FlexEnvelope::cancelRelease(unsigned delay)
{
    UNUSED(delay);
    Impl& impl = *impl_;
    const FlexEGDescription& desc = *impl.desc_;

    if (impl.currentFramesUntilRelease_) {
        // Cancel the future release
        impl.currentFramesUntilRelease_ = absl::nullopt;
        return;
    }

    if (!impl.isReleased_)
        return;

    impl.isReleased_ = false;
    impl.advanceToStage(desc.sustain);
    impl.stageTargetLevel_ = impl.currentLevel_;
}

unsigned FlexEnvelope::getRemainingDelay() const noexcept
{
    const Impl& impl = *impl_;
    return static_cast<unsigned>(impl.delayFramesLeft_);
}

bool FlexEnvelope::isReleased() const noexcept
{
    const Impl& impl = *impl_;
    return impl.isReleased_;
}

bool FlexEnvelope::isFinished() const noexcept
{
    const Impl& impl = *impl_;
    const FlexEGDescription& desc = *impl.desc_;
    return impl.currentStageNumber_ >= desc.points.size();
}

void FlexEnvelope::process(absl::Span<float> out)
{
    Impl& impl = *impl_;
    if (impl.desc_->dynamic) {
        int processed = 0;
        int remaining = static_cast<int>(out.size());
        while(remaining > 0) {
            impl.updateCurrentTimeAndLevel(processed);
            int chunkSize = min(config::processChunkSize, remaining);
            impl.process(out.subspan(processed, chunkSize));
            processed += chunkSize;
            remaining -= chunkSize;
        }
    } else {
        impl.process(out);
    }
}

void FlexEnvelope::Impl::process(absl::Span<float> out)
{
    const FlexEGDescription& desc = *desc_;
    size_t numFrames = out.size();
    const float samplePeriod = samplePeriod_;

    // Skip the initial delay, for frame-accurate trigger
    size_t skipFrames = std::min(numFrames, delayFramesLeft_);
    if (skipFrames > 0) {
        delayFramesLeft_ -= skipFrames;
        fill(absl::MakeSpan(out.data(), skipFrames), 0.0f);
        out.remove_prefix(skipFrames);
        numFrames -= skipFrames;
    }

    // Envelope finished?
    if (currentStageNumber_ >= desc.points.size()) {
        fill(out, 0.0f);
        return;
    }

    size_t frameIndex = 0;

    while (frameIndex < numFrames) {
        // Check for release
        if (currentFramesUntilRelease_ && *currentFramesUntilRelease_ == 0) {
            isReleased_ = true;
            currentFramesUntilRelease_ = absl::nullopt;
        }

        // Perform stage transitions
        if (isReleased_) {
            // on release, fast forward past the sustain stage
            const unsigned sustainStage = desc.sustain;
            while (currentStageNumber_ <= sustainStage) {
                if (!advanceToNextStage()) {
                    out.remove_prefix(frameIndex);
                    fill(out, 0.0f);
                    return;
                }
            }
        }
        while ((!stageSustained_ || freeRunning_) && currentTime_ >= stageTime_) {
            // advance through completed timed stages
            ASSERT(isReleased_ || !stageSustained_ || freeRunning_);
            if (stageTime_ == 0) {
                // if stage is of zero duration, immediate transition to level
                currentLevel_ = stageTargetLevel_;
            }
            if (!advanceToNextStage()) {
                out.remove_prefix(frameIndex);
                fill(out, 0.0f);
                return;
            }
        }

        // Process without going past the release point, if there is one
        size_t maxFrameIndex = numFrames;
        if (currentFramesUntilRelease_)
            maxFrameIndex = std::min(maxFrameIndex, frameIndex + *currentFramesUntilRelease_);

        // Process the current stage
        float time = currentTime_;
        float level = currentLevel_;
        const float stageEndTime = stageTime_;
        const float sourceLevel = stageSourceLevel_;
        const float targetLevel = stageTargetLevel_;
        const bool sustained = stageSustained_;
        const Curve& curve = *stageCurve_;
        size_t framesDone = 0;
        while ((time < stageEndTime || sustained) && frameIndex < maxFrameIndex) {
            time += samplePeriod;
            float x = time * (1.0f / stageEndTime);
            float c = curve.evalNormalized(x);
            level = sourceLevel + c * (targetLevel - sourceLevel);
            out[frameIndex++] = level;
            ++framesDone;
        }
        currentLevel_ = level;

        // Update the counter to release
        if (currentFramesUntilRelease_)
            *currentFramesUntilRelease_ -= framesDone;

        currentTime_ = time;
    }
}

bool FlexEnvelope::Impl::advanceToStage(unsigned stageNumber)
{
    const FlexEGDescription& desc = *desc_;
    currentStageNumber_ = stageNumber;

    if (stageNumber >= desc.points.size())
        return false;

    const FlexEGPoint& point = desc.points[stageNumber];
    stageSourceLevel_ = currentLevel_;
    currentTime_ = 0.0f;
    updateCurrentTimeAndLevel();
    stageSustained_ = int(stageNumber) == desc.sustain;
    stageCurve_ = &point.curve();

    return true;
};

bool FlexEnvelope::Impl::advanceToNextStage()
{
    return advanceToStage(currentStageNumber_ + 1);
}

void FlexEnvelope::Impl::updateCurrentTimeAndLevel(int delay)
{
    const FlexEGDescription& desc = *desc_;
    if (currentStageNumber_ >= desc.points.size())
        return;

    const FlexEGPoint& point = desc.points[currentStageNumber_];
    const MidiState& midiState = resources_->getMidiState();
    stageTargetLevel_ = point.getLevel(midiState, delay);
    stageTime_ = point.getTime(midiState, delay);
}

} // namespace sfz
