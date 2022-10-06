// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once

#include "Range.h"
#include "Defaults.h"
#include "SfzHelpers.h"
#include "absl/types/span.h"

namespace sfz {
/**
 * @brief Compute a crossfade in value with respect to a crossfade range (note, velocity, cc, ...)
 */
template <class T, bool C, class U>
float crossfadeIn(const sfz::Range<T, C>& crossfadeRange, U value, CrossfadeCurve curve)
{
    constexpr float gapOffset { static_cast<T>(normalize7Bits(1)) };
    if (value < crossfadeRange.getStart())
        return 0.0f;

    const auto length = static_cast<float>(crossfadeRange.length()) - gapOffset;
    if (length <= 0.0f)
        return 1.0f;

    else if (value < crossfadeRange.getEnd()) {
        const auto distanceFromStart = static_cast<float>(value - crossfadeRange.getStart());
        const auto crossfadePosition = distanceFromStart / length;
        if (curve == CrossfadeCurve::power)
            return sqrt(crossfadePosition);
        if (curve == CrossfadeCurve::gain)
            return crossfadePosition;
    }

    return 1.0f;
}

/**
 * @brief Compute a crossfade out value with respect to a crossfade range (note, velocity, cc, ...)
 */
template <class T, bool C, class U>
float crossfadeOut(const sfz::Range<T, C>& crossfadeRange, U value, CrossfadeCurve curve)
{
    constexpr float gapOffset { static_cast<T>(normalize7Bits(1)) };
    const auto length = static_cast<float>(crossfadeRange.length()) - gapOffset;
    if (length <= 0.0f)
        return 1.0f;

    else if (value > crossfadeRange.getStart()) {
        const auto distanceFromStart = static_cast<float>(value - crossfadeRange.getStart());
        const auto crossfadePosition = distanceFromStart / length;
        if (crossfadePosition > 1.0f)
            return 0.0f;
        if (curve == CrossfadeCurve::power)
            return std::sqrt(1 - crossfadePosition);
        if (curve == CrossfadeCurve::gain)
            return 1 - crossfadePosition;
    }

    return 1.0f;
}

/**
 * @brief Compute a linear envelope based on events, with a lambda applied to the event values
 *
 * @tparam F
 * @param events
 * @param envelope
 * @param lambda
 */
template <class F>
void linearEnvelope(const EventVector& events, absl::Span<float> envelope, F&& lambda)
{
    ASSERT(events.size() > 0);
    ASSERT(events[0].delay == 0);

    if (envelope.size() == 0)
        return;

    const auto maxDelay = static_cast<int>(envelope.size() - 1);

    auto lastValue = lambda(events[0].value);
    auto lastDelay = events[0].delay;
    for (unsigned i = 1; i < events.size() && lastDelay < maxDelay; ++i) {
        const auto length = min(events[i].delay, maxDelay) - lastDelay;
        const auto step = (lambda(events[i].value) - lastValue) / length;
        lastValue = linearRamp<float>(envelope.subspan(lastDelay, length), lastValue, step);
        lastDelay += length;
    }
    fill(envelope.subspan(lastDelay), lastValue);
}

/**
 * @brief Compute a quantized linear envelope based on events, with a lambda applied to the event values
 *
 * @tparam F
 * @param events
 * @param envelope
 * @param lambda
 * @param step
 */
template <class F>
void linearEnvelope(const EventVector& events, absl::Span<float> envelope, F&& lambda, float step)
{
    ASSERT(events.size() > 0);
    ASSERT(events[0].delay == 0);

    if (step == 0.0f) {
        linearEnvelope(events, envelope, std::forward<F>(lambda));
        return;
    }

    if (envelope.size() == 0)
        return;

    auto quantize = [step](float value) -> float {
        return std::trunc(value / step) * step;
    };
    const auto maxDelay = static_cast<int>(envelope.size() - 1);

    auto lastValue = quantize(lambda(events[0].value));
    auto lastDelay = events[0].delay;
    for (unsigned i = 1; i < events.size() && lastDelay < maxDelay; ++i) {
        const auto nextValue = quantize(lambda(events[i].value));
        const auto difference = std::abs(nextValue - lastValue);
        const auto length = min(events[i].delay, maxDelay) - lastDelay;

        if (difference < step) {
            fill(envelope.subspan(lastDelay, length), lastValue);
            lastValue = nextValue;
            lastDelay += length;
            continue;
        }

        const auto numSteps = static_cast<int>(difference / step);
        const auto stepLength = static_cast<int>(length / numSteps);
        for (int i = 0; i < numSteps; ++i) {
            fill(envelope.subspan(lastDelay, stepLength), lastValue);
            lastValue += lastValue <= nextValue ? step : -step;
            lastDelay += stepLength;
        }
    }
    fill(envelope.subspan(lastDelay), lastValue);
}


/**
 * @brief Compute a multiplicative envelope based on events, with a lambda applied to the event values
 *
 * @tparam F
 * @param events
 * @param envelope
 * @param lambda
 */
template <class F>
void multiplicativeEnvelope(const EventVector& events, absl::Span<float> envelope, F&& lambda)
{
    ASSERT(events.size() > 0);
    ASSERT(events[0].delay == 0);

    if (envelope.size() == 0)
        return;

    const auto maxDelay = static_cast<int>(envelope.size() - 1);

    auto lastValue = lambda(events[0].value);
    auto lastDelay = events[0].delay;
    for (unsigned i = 1; i < events.size() && lastDelay < maxDelay; ++i) {
        const auto length = min(events[i].delay, maxDelay) - lastDelay;
        const auto nextValue = lambda(events[i].value);
        const auto step = std::exp((std::log(nextValue) - std::log(lastValue)) / length);
        multiplicativeRamp<float>(envelope.subspan(lastDelay, length), lastValue, step);
        lastValue = nextValue;
        lastDelay += length;
    }
    fill(envelope.subspan(lastDelay), lastValue);
}

/**
 * @brief Compute a quantized multiplicative envelope based on events, with a lambda applied to the event values
 *
 * @tparam F
 * @tparam Round is true if the quantization rounds rather than truncs the elements
 * @param events
 * @param envelope
 * @param lambda
 * @param step
 */
template <class F, bool Round = false>
void multiplicativeEnvelope(const EventVector& events, absl::Span<float> envelope, F&& lambda, float step)
{
    ASSERT(events.size() > 0);
    ASSERT(events[0].delay == 0);
    ASSERT(step != 0.0f);

    ScopedRoundingMode roundingMode { Round ? FE_TONEAREST : FE_TOWARDZERO };

    if (envelope.size() == 0)
        return;
    const auto maxDelay = static_cast<int>(envelope.size() - 1);
    const auto logStep = std::log(step);
    // If we assume that a = b.q^r for b in (1, q) then
    // log a     log b
    // -----  =  -----  +  r
    // log q     log q
    // and log(b)\log(q) is between 0 and 1.
    auto quantize = [logStep](float value) -> float {
        return std::exp(logStep * std::rint(std::log(value) / logStep));
    };

    auto lastValue = quantize(lambda(events[0].value));
    auto lastDelay = events[0].delay;
    for (unsigned i = 1; i < events.size() && lastDelay < maxDelay; ++i) {
        const auto length = min(events[i].delay, maxDelay) - lastDelay;
        const auto nextValue = quantize(lambda(events[i].value));
        const auto difference = nextValue > lastValue ? nextValue / lastValue : lastValue / nextValue;

        if (difference < step) {
            fill(envelope.subspan(lastDelay, length), lastValue);
            lastValue = nextValue;
            lastDelay += length;
            continue;
        }

        const auto numSteps = std::round(std::log(difference) / logStep);
        const auto stepLength = static_cast<int>(length / numSteps);
        for (int i = 0; i < static_cast<int>(numSteps); ++i) {
            fill(envelope.subspan(lastDelay, stepLength), lastValue);
            lastValue = nextValue > lastValue ? lastValue * step : lastValue / step;
            lastDelay += stepLength;
        }
    }
    fill(envelope.subspan(lastDelay), lastValue);
}

/**
 * @brief Alias for a multiplicative envelope that rounds the elements
 *
 * @tparam F
 * @param events
 * @param envelope
 * @param lambda
 * @param step
 */
template <class F>
void pitchBendEnvelope(const EventVector& events, absl::Span<float> envelope, F&& lambda, float step)
{
    multiplicativeEnvelope<F, true>(events, envelope, std::forward<F>(lambda), step);
}

/**
 * @brief Alias for a multiplicative envelope
 *
 * @tparam F
 * @param events
 * @param envelope
 * @param lambda
 */
template <class F>
void pitchBendEnvelope(const EventVector& events, absl::Span<float> envelope, F&& lambda)
{
    multiplicativeEnvelope<F>(events, envelope, std::forward<F>(lambda));
}

} // namespace sfz
