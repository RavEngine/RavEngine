// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once

namespace sfz {

enum InterpolatorModel : int {
    // a nearest interpolator
    kInterpolatorNearest,
    // a linear interpolator
    kInterpolatorLinear,
    // a Hermite 3rd order interpolator
    kInterpolatorHermite3,
    // a B-spline 3rd order interpolator
    kInterpolatorBspline3,
    // a windowed-sinc 8-point interpolator
    kInterpolatorSinc8,
    // a windowed-sinc 12-point interpolator
    kInterpolatorSinc12,
    // a windowed-sinc 16-point interpolator
    kInterpolatorSinc16,
    // a windowed-sinc 24-point interpolator
    kInterpolatorSinc24,
    // a windowed-sinc 36-point interpolator
    kInterpolatorSinc36,
    // a windowed-sinc 48-point interpolator
    kInterpolatorSinc48,
    // a windowed-sinc 60-point interpolator
    kInterpolatorSinc60,
    // a windowed-sinc 72-point interpolator
    kInterpolatorSinc72,
};

/**
 * @brief Initialize interpolators
 *
 * This precomputes windowed-sinc tables globally.
 * It needs to be called at least once, before using the windowed-sinc models.
 *
 * These are not computed at static initialization time, to prevent slowing down
 * an audio plugin library scan (eg. VST). The static-local-variable method is
 * avoided also, because we don't want this overhead on a frame-by-frame basis.
 */
void initializeInterpolators();

/**
 * @brief Interpolate from a vector of values
 *
 * @tparam M the interpolator model
 * @tparam R the sample type
 * @param values Pointer to a value in a larger vector of values.
 *               Depending on the interpolator the algorithm may
 *               read samples before and after. Usually you need
 *               to ensure that you have order - 1 samples available
 *               before and after the pointer, padding if necessary.
 * @param coeff the interpolation coefficient
 * @return R
 */
template <InterpolatorModel M, class R>
R interpolate(const R* values, R coeff);

} // namespace sfz

#include "Interpolators.hpp"
