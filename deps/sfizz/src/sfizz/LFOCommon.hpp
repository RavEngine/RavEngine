// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "LFOCommon.h"
#include <cmath>

namespace sfz {
namespace lfo {

    // Pulse and Square levels
    static constexpr float loPulse = 0.0f; // 0 in ARIA, -1 in Cakewalk
    static constexpr float hiPulse = 1.0f;

    template <>
    inline float evaluateAtPhase<LFOWave::Triangle>(float phase)
    {
        float y = -4 * phase + 2;
        y = (phase < 0.25f) ? (4 * phase) : y;
        y = (phase > 0.75f) ? (4 * phase - 4) : y;
        return y;
    }

    template <>
    inline float evaluateAtPhase<LFOWave::Sine>(float phase)
    {
        float x = phase + phase - 1;
        return -4 * x * (1 - std::fabs(x));
    }

    template <>
    inline float evaluateAtPhase<LFOWave::Pulse75>(float phase)
    {
        return (phase < 0.75f) ? hiPulse : loPulse;
    }

    template <>
    inline float evaluateAtPhase<LFOWave::Square>(float phase)
    {
        return (phase < 0.5f) ? hiPulse : loPulse;
    }

    template <>
    inline float evaluateAtPhase<LFOWave::Pulse25>(float phase)
    {
        return (phase < 0.25f) ? hiPulse : loPulse;
    }

    template <>
    inline float evaluateAtPhase<LFOWave::Pulse12_5>(float phase)
    {
        return (phase < 0.125f) ? hiPulse : loPulse;
    }

    template <>
    inline float evaluateAtPhase<LFOWave::Ramp>(float phase)
    {
        return 2 * phase - 1;
    }

    template <>
    inline float evaluateAtPhase<LFOWave::Saw>(float phase)
    {
        return 1 - 2 * phase;
    }

} // namespace lfo
} // namespace sfz
