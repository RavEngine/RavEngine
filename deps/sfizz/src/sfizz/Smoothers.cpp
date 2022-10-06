// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Smoothers.h"
#include "Config.h"
#include "MathHelpers.h"
#include "SfzHelpers.h"
#include "SIMDHelpers.h"
#include <simde/simde-features.h>
#if SIMDE_NATURAL_VECTOR_SIZE_GE(128)
#include <simde/x86/sse.h>
#endif

namespace sfz {

OnePoleSmoother::OnePoleSmoother()
{
}

void OnePoleSmoother::setSmoothing(unsigned smoothValue, float sampleRate)
{
    smoothing = (smoothValue > 0);
    if (smoothing) {
        filter.setGain(std::tan(1.0f / (2 * config::smoothTauPerStep * smoothValue * sampleRate)));
    }
}

void OnePoleSmoother::reset(float value)
{
    filter.reset(value);
    target_ = value;
}

void OnePoleSmoother::resetToTarget()
{
    reset(target_);
}

void OnePoleSmoother::process(absl::Span<const float> input, absl::Span<float> output, bool canShortcut)
{
    CHECK_SPAN_SIZES(input, output);
    if (input.size() == 0)
        return;

    if (canShortcut) {
        float in = input.front();
        float rel = std::abs(in - current()) / (std::abs(in) + config::virtuallyZero);
        canShortcut = rel < config::smoothingShortcutThreshold;
    }

    if (canShortcut) {
        if (input.data() != output.data())
            copy<float>(input, output);

        filter.reset(input.back());
    } else if (smoothing) {
        filter.processLowpass(input, output);
    } else if (input.data() != output.data()) {
        copy<float>(input, output);
    }

    target_ = input.back();
}

///
LinearSmoother::LinearSmoother()
{
}

void LinearSmoother::setSmoothing(unsigned smoothValue, float sampleRate)
{
    const float smoothTime = 1e-3f * smoothValue;
    smoothFrames_ = static_cast<int32_t>(smoothTime * sampleRate);
}

void LinearSmoother::reset(float value)
{
    current_ = value;
    target_ = value;
    step_ = 0.0;
    //framesToTarget_ = 0;
}

void LinearSmoother::resetToTarget()
{
    reset(target_);
}

void LinearSmoother::process(absl::Span<const float> input, absl::Span<float> output, bool canShortcut)
{
    CHECK_SPAN_SIZES(input, output);

    uint32_t i = 0;
    const uint32_t count = static_cast<uint32_t>(input.size());
    if (count == 0)
        return;

    float current = current_;
    float target = target_;
    const int32_t smoothFrames = smoothFrames_;

    if (smoothFrames < 2 || (canShortcut && current == target && current == input.front())) {
        if (input.data() != output.data())
            copy<float>(input, output);
        reset(input.back());
        return;
    }

    float step = step_;
    // int32_t framesToTarget = framesToTarget_;

#if SIMDE_NATURAL_VECTOR_SIZE_GE(128)
    for (; i + 15 < count; i += 16) {
        const float nextTarget = input[i + 15];
        if (target != nextTarget) {
            target = nextTarget;
            //framesToTarget = (framesToTarget > 0) ? framesToTarget : smoothFrames;
            //step = (target - current) / max(16, framesToTarget);
            step = (target - current) / max(16, smoothFrames);
        }
        const simde__m128 targetX4 = simde_mm_set1_ps(target);
        if (target > current) {
            simde__m128 stepX4 = simde_mm_set1_ps(step);
            simde__m128 tmp1X4 = simde_mm_mul_ps(stepX4, simde_mm_setr_ps(1.0f, 2.0f, 3.0f, 4.0f));
            simde__m128 tmp2X4 = simde_mm_shuffle_ps(tmp1X4, tmp1X4, SIMDE_MM_SHUFFLE(3, 3, 3, 3));
            simde__m128 current1X4 = simde_mm_add_ps(simde_mm_set1_ps(current), tmp1X4);
            simde_mm_storeu_ps(&output[i], simde_mm_min_ps(current1X4, targetX4));
            simde__m128 current2X4 = simde_mm_add_ps(current1X4, tmp2X4);
            simde_mm_storeu_ps(&output[i + 4], simde_mm_min_ps(current2X4, targetX4));
            simde__m128 current3X4 = simde_mm_add_ps(current2X4, tmp2X4);
            simde_mm_storeu_ps(&output[i + 8], simde_mm_min_ps(current3X4, targetX4));
            simde__m128 current4X4 = simde_mm_add_ps(current3X4, tmp2X4);
            simde__m128 limited4X4 = simde_mm_min_ps(current4X4, targetX4);
            simde_mm_storeu_ps(&output[i + 12], limited4X4);
            current = simde_mm_cvtss_f32(simde_mm_shuffle_ps(limited4X4, limited4X4, SIMDE_MM_SHUFFLE(3, 3, 3, 3)));
        }
        else if (target < current) {
            simde__m128 stepX4 = simde_mm_set1_ps(step);
            simde__m128 tmp1X4 = simde_mm_mul_ps(stepX4, simde_mm_setr_ps(1.0f, 2.0f, 3.0f, 4.0f));
            simde__m128 tmp2X4 = simde_mm_shuffle_ps(tmp1X4, tmp1X4, SIMDE_MM_SHUFFLE(3, 3, 3, 3));
            simde__m128 current1X4 = simde_mm_add_ps(simde_mm_set1_ps(current), tmp1X4);
            simde_mm_storeu_ps(&output[i], simde_mm_max_ps(current1X4, targetX4));
            simde__m128 current2X4 = simde_mm_add_ps(current1X4, tmp2X4);
            simde_mm_storeu_ps(&output[i + 4], simde_mm_max_ps(current2X4, targetX4));
            simde__m128 current3X4 = simde_mm_add_ps(current2X4, tmp2X4);
            simde_mm_storeu_ps(&output[i + 8], simde_mm_max_ps(current3X4, targetX4));
            simde__m128 current4X4 = simde_mm_add_ps(current3X4, tmp2X4);
            simde__m128 limited4X4 = simde_mm_max_ps(current4X4, targetX4);
            simde_mm_storeu_ps(&output[i + 12], limited4X4);
            current = simde_mm_cvtss_f32(simde_mm_shuffle_ps(limited4X4, limited4X4, SIMDE_MM_SHUFFLE(3, 3, 3, 3)));
        }
        else {
            simde_mm_storeu_ps(&output[i], targetX4);
            simde_mm_storeu_ps(&output[i + 4], targetX4);
            simde_mm_storeu_ps(&output[i + 8], targetX4);
            simde_mm_storeu_ps(&output[i + 12], targetX4);
        }
        //framesToTarget -= 16;
    }
#else
    for (; i + 15 < count; i += 16) {
        const float nextTarget = input[i + 15];

        if (target != nextTarget) {
            target = nextTarget;
            //framesToTarget = (framesToTarget > 0) ? framesToTarget : smoothFrames;
            //step = (target - current) / max(16, framesToTarget);
            step = (target - current) / max(16, smoothFrames);
        }
        if (target > current) {
            for (size_t j = 0; j < 16; ++j)
                output[i + j] = current = min(target, current + step);
        }
        else if (target < current) {
            for (size_t j = 0; j < 16; ++j)
                output[i + j] = current = max(target, current + step);
        }
        else {
            for (size_t j = 0; j < 16; ++j)
                output[i + j] = target;
        }

        //framesToTarget -= 16;
    }
#endif

    if (i < count) {
        const float nextTarget = input[count - 1];
        if (target != nextTarget) {
            target = nextTarget;
            // framesToTarget = (framesToTarget > 0) ? framesToTarget : smoothFrames;
            // step = (target - current) / max(static_cast<int32_t>(count - i), framesToTarget);
            step = (target - current) / max(static_cast<int32_t>(count - i), smoothFrames);
        }
        if (target > current) {
            for (; i < count; ++i)
                output[i] = current = min(target, current + step);
        }
        else if (target < current) {
            for (; i < count; ++i)
                output[i] = current = max(target, current + step);
        }
        else {
            for (; i < count; ++i)
                output[i] = target;
        }
        //framesToTarget -= count;
    }

    current_ = current;
    target_ = target;
    step_ = step;
    //framesToTarget_ = max(0, framesToTarget);
}

}
