// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once

namespace sfz {

enum class LFOWave : int {
    Triangle,
    Sine,
    Pulse75,
    Square,
    Pulse25,
    Pulse12_5,
    Ramp,
    Saw,
    // ARIA extra
    RandomSH = 12,
};

namespace lfo {

template <LFOWave Wave> float evaluateAtPhase(float phase);

} // namespace lfo
} // namespace sfz

#include "LFOCommon.hpp"
