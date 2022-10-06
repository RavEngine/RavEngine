// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

/**
 * @file SIMDHelpers.h
 * @brief This file contains useful functions to treat buffers of numerical values
 * (e.g. a buffer of floats usually).
 *
 * These functions are templated to apply on
 * various underlying buffer types, and this file contains the generic version of the
 * function. Some templates specializations exists for different architecture that try
 * to make use of SIMD intrinsics in SIMDHelpers.cpp. A runtime dispatch can help if you
 * write implementation for larger/newer SIMD operations.
 *
 * All the SIMD functions are benchmarked. If you run the benchmark for a given function you can check
 * if it is interesting to run the SIMD version by default. The interest is that you can activate
 * and deactivate each SIMD specialization with a fine granularity, since SIMD performance
 * will be very dependent on the processor architecture. Modern processors can also organize their
 * instructions so that scalar non-SIMD code runs sometimes much more efficiently than SIMD code
 * especially when the latter does not operate on misaligned buffers.
 *
 */
#pragma once
#include "Config.h"
#include "Range.h"
#include "MathHelpers.h"
#include "utility/Debug.h"
#include "simd/HelpersScalar.h"
#include <absl/types/span.h>
#include <array>
#include <cmath>

namespace sfz {

// NOTE: The goal is to collapse all checks on release, and compute the sentinels for the span versions
// https://godbolt.org/z/43dveW shows this should work

enum class SIMDOps {
    writeInterleaved,
    readInterleaved,
    fill,
    gain,
    gain1,
    divide,
    linearRamp,
    multiplicativeRamp,
    add,
    add1,
    subtract,
    subtract1,
    multiplyAdd,
    multiplyAdd1,
    multiplyMul,
    multiplyMul1,
    copy,
    cumsum,
    diff,
    sfzInterpolationCast,
    mean,
    sumSquares,
    upsampling,
    clampAll,
    allWithin,
    _sentinel //
};

// Call this at least once before using SIMD operations
void initializeSIMDDispatchers();

// Enable or disable SIMD accelerators at runtime
template<class T>
void resetSIMDOpStatus();

template<class T>
void setSIMDOpStatus(SIMDOps op, bool status);

template<class T>
bool getSIMDOpStatus(SIMDOps op);

// Float specializations
template<>
void resetSIMDOpStatus<float>();

template<>
void setSIMDOpStatus<float>(SIMDOps op, bool status);

template<>
bool getSIMDOpStatus<float>(SIMDOps op);

/**
 * @brief Read interleaved stereo data from a buffer and separate it in a left/right pair of buffers.
 *
 * @param input
 * @param outputLeft
 * @param outputRight
 * @param inputSize
 */
void readInterleaved(const float* input, float* outputLeft, float* outputRight, unsigned inputSize) noexcept;

inline void readInterleaved(absl::Span<const float> input, absl::Span<float> outputLeft, absl::Span<float> outputRight) noexcept
{
    // Something is fishy with the sizes
    SFIZZ_CHECK(outputLeft.size() == input.size() / 2);
    SFIZZ_CHECK(outputRight.size() == input.size() / 2);
    const auto size = min(input.size(), 2 * outputLeft.size(), 2 * outputRight.size());
    readInterleaved(input.data(), outputLeft.data(), outputRight.data(), size);
}

/**
 * @brief Write a pair of left and right stereo input into a single buffer interleaved.
 *
 * @param inputLeft
 * @param inputRight
 * @param output
 * @param outputSize
 */
void writeInterleaved(const float* inputLeft, const float* inputRight, float* output, unsigned outputSize) noexcept;

inline void writeInterleaved(absl::Span<const float> inputLeft, absl::Span<const float> inputRight, absl::Span<float> output) noexcept
{
    // Something is fishy with the sizes
    SFIZZ_CHECK(inputLeft.size() == output.size() / 2);
    SFIZZ_CHECK(inputRight.size() == output.size() / 2);
    const auto size = min(output.size(), 2 * inputLeft.size(), 2 * inputRight.size());
    writeInterleaved(inputLeft.data(), inputRight.data(), output.data(), size);
}

/**
 * @brief Fill a buffer with a value
 *
 * @tparam T the underlying type
 * @param output
 * @param value
 */
template <class T>
void fill(T* output, T value, unsigned size) noexcept
{
    std::fill(output, output + size, value);
}

template <class T>
void fill(absl::Span<T> output, T value) noexcept
{
    fill<T>(output.data(), value, output.size());
}

/**
 * @brief Applies a scalar gain to the input
 *
 * @param gain the gain to apply
 * @param input
 * @param output
 * @param size
 */
template<class T>
void applyGain1(T gain, const T* input, T* output, unsigned size) noexcept
{
    gain1Scalar(gain, input, output, size);
}

template<>
void applyGain1<float>(float gain, const float* input, float* output, unsigned size) noexcept;

template<class T>
inline void applyGain1(T gain, absl::Span<const T> input, absl::Span<T> output) noexcept
{
    CHECK_SPAN_SIZES(input, output);
    applyGain1<T>(gain, input.data(), output.data(), minSpanSize(input, output));
}

/**
 * @brief Applies a scalar gain inplace
 *
 * @param gain the gain to apply
 * @param array
 * @param size
 */
template<class T>
inline void applyGain1(T gain, T* array, unsigned size) noexcept
{
    applyGain1<T>(gain, array, array, size);
}

template<class T>
inline void applyGain1(T gain, absl::Span<T> array) noexcept
{
    applyGain1<T>(gain, array.data(), array.data(), array.size());
}

/**
 * @brief Applies a vector gain to an input span
 *
 * @param gain
 * @param input
 * @param output
 * @param size
 */
template<class T>
void applyGain(const T* gain, const T* input, T* output, unsigned size) noexcept
{
    gainScalar(gain, input, output, size);
}

template<>
void applyGain<float>(const float* gain, const float* input, float* output, unsigned size) noexcept;

template<class T>
inline void applyGain(absl::Span<const T> gain, absl::Span<const T> input, absl::Span<T> output) noexcept
{
    CHECK_SPAN_SIZES(gain, input, output);
    applyGain<T>(gain.data(), input.data(), output.data(), minSpanSize(gain, input, output));
}

/**
 * @brief Applies a vector gain in-place on a span
 *
 * @param gain
 * @param array
 * @param size
 */
template<class T>
inline void applyGain(const T* gain, T* array, unsigned size) noexcept
{
    applyGain<T>(gain, array, array, size);
}

template<class T>
inline void applyGain(absl::Span<const T> gain, absl::Span<T> array) noexcept
{
    CHECK_SPAN_SIZES(gain, array);
    applyGain<T>(gain.data(), array.data(), array.data(), minSpanSize(gain, array));
}

/**
 * @brief Divide a vector by another vector
 *
 * The output size will be the minimum of the divisor, input span and output span size.
 *
 * @tparam T the underlying type
 * @param input
 * @param divisor
 * @param output
 * @param size
 */
template <class T>
void divide(const T* input, const T* divisor, T* output, unsigned size) noexcept
{
    divideScalar(input, divisor, output, size);
}

template <>
void divide<float>(const float* input, const float* divisor, float* output, unsigned size) noexcept;

template <class T>
inline void divide(absl::Span<const T> input, absl::Span<const T> divisor, absl::Span<T> output) noexcept
{
    CHECK_SPAN_SIZES(input, divisor, output);
    divide<T>(input.data(), divisor.data(), output.data(), minSpanSize(input, divisor, output));
}

/**
 * @brief Divide a vector by another in place
 *
 * @tparam T the underlying type
 * @param output
 * @param divisor
 */
template <class T>
void divide(absl::Span<T> output,  absl::Span<const T> divisor) noexcept
{
    CHECK_SPAN_SIZES(divisor, output);
    divide<T>(output.data(), divisor.data(), output.data(), minSpanSize(divisor, output));
}

/**
 * @brief Applies a gain to the input and add it on the output
 *
 * @tparam T the underlying type
 * @param gain
 * @param input
 * @param output
 * @param size
 */
template <class T>
void multiplyAdd(const T* gain, const T* input, T* output, unsigned size) noexcept
{
    multiplyAddScalar(gain, input, output, size);
}

template <>
void multiplyAdd<float>(const float* gain, const float* input, float* output, unsigned size) noexcept;

template <class T>
void multiplyAdd(absl::Span<const T> gain, absl::Span<const T> input, absl::Span<T> output) noexcept
{
    CHECK_SPAN_SIZES(gain, input, output);
    multiplyAdd<T>(gain.data(), input.data(), output.data(), minSpanSize(gain, input, output));
}

/**
 * @brief Applies a gain to the input and add it on the output
 *
 * @tparam T the underlying type
 * @param gain
 * @param input
 * @param output
 * @param size
 */
template <class T>
void multiplyAdd1(T gain, const T* input, T* output, unsigned size) noexcept
{
    multiplyAdd1Scalar(gain, input, output, size);
}

template <>
void multiplyAdd1<float>(float gain, const float* input, float* output, unsigned size) noexcept;

template <class T>
void multiplyAdd1(T gain, absl::Span<const T> input, absl::Span<T> output) noexcept
{
    CHECK_SPAN_SIZES(input, output);
    multiplyAdd1<T>(gain, input.data(), output.data(), minSpanSize(input, output));
}

/**
 * @brief Applies a gain to the input and multiply the output with it
 *
 * @tparam T the underlying type
 * @param gain
 * @param input
 * @param output
 * @param size
 */
template <class T>
void multiplyMul(const T* gain, const T* input, T* output, unsigned size) noexcept
{
    multiplyMulScalar(gain, input, output, size);
}

template <>
void multiplyMul<float>(const float* gain, const float* input, float* output, unsigned size) noexcept;

template <class T>
void multiplyMul(absl::Span<const T> gain, absl::Span<const T> input, absl::Span<T> output) noexcept
{
    CHECK_SPAN_SIZES(gain, input, output);
    multiplyMul<T>(gain.data(), input.data(), output.data(), minSpanSize(gain, input, output));
}

/**
 * @brief Applies a fixed gain to the input and multiply the output with it
 *
 * @tparam T the underlying type
 * @param gain
 * @param input
 * @param output
 * @param size
 */
template <class T>
void multiplyMul1(T gain, const T* input, T* output, unsigned size) noexcept
{
    multiplyMul1Scalar(gain, input, output, size);
}

template <>
void multiplyMul1<float>(float gain, const float* input, float* output, unsigned size) noexcept;

template <class T>
void multiplyMul1(T gain, absl::Span<const T> input, absl::Span<T> output) noexcept
{
    CHECK_SPAN_SIZES(input, output);
    multiplyMul1<T>(gain, input.data(), output.data(), minSpanSize(input, output));
}

/**
 * @brief Compute a linear ramp blockwise between 2 values
 *
 * @tparam T the underlying type
 * @param output The destination span
 * @param start
 * @param step
 * @param size
 * @return T
 */
template <class T>
T linearRamp(T* output, T start, T step, unsigned size) noexcept
{
    linearRampScalar(output, start, step, size);
}

template <>
float linearRamp<float>(float* output, float start, float step, unsigned size) noexcept;

template <class T>
T linearRamp(absl::Span<T> output, T start, T step) noexcept
{
    return linearRamp(output.data(), start, step, output.size());
}

/**
 * @brief Compute a multiplicative ramp blockwise between 2 values
 *
 * @tparam T the underlying type
 * @param output The destination span
 * @param start
 * @param step
 * @return T
 */
template <class T>
T multiplicativeRamp(T* output, T start, T step, unsigned size) noexcept
{
    multiplicativeRampScalar(output, start, step, size);
}

template <>
float multiplicativeRamp<float>(float* output, float start, float step, unsigned size) noexcept;

template <class T>
T multiplicativeRamp(absl::Span<T> output, T start, T step) noexcept
{
    return multiplicativeRamp(output.data(), start, step, output.size());

}

/**
 * @brief Add an input span to the output span
 *
 * @tparam T the underlying type
 * @param input
 * @param output
 * @param size
 */
template <class T>
void add(const T* input, T* output, unsigned size) noexcept
{
    addScalar(input, output, size);
}

template <>
void add<float>(const float* input, float* output, unsigned size) noexcept;

template <class T>
void add(absl::Span<const T> input, absl::Span<T> output) noexcept
{
    CHECK_SPAN_SIZES(input, output);
    add<T>(input.data(), output.data(), minSpanSize(input, output));
}

/**
 * @brief Add a value inplace
 *
 * @tparam T the underlying type
 * @param value
 * @param output
 * @param size
 */
template <class T>
void add1(T value, T* output, unsigned size) noexcept
{
    add1Scalar(value, output, size);
}

template <>
void add1<float>(float value, float* output, unsigned size) noexcept;

template <class T>
void add1(T value, absl::Span<T> output) noexcept
{
    add1<T>(value, output.data(), output.size());
}

/**
 * @brief Subtract an input span from the output span
 *
 * @tparam T the underlying type
 * @param input
 * @param output
 * @param size
 */
template <class T>
void subtract(const T* input, T* output, unsigned size) noexcept
{
    subtractScalar(input, output, size);
}

template <>
void subtract<float>(const float* input, float* output, unsigned size) noexcept;

template <class T>
void subtract(absl::Span<const T> input, absl::Span<T> output) noexcept
{
    CHECK_SPAN_SIZES(input, output);
    subtract<T>(input.data(), output.data(), minSpanSize(input, output));
}

/**
 * @brief Subtract a value inplace
 *
 * @tparam T the underlying type
 * @param value
 * @param output
 * @param size
 */
template <class T>
void subtract1(T value, T* output, unsigned size) noexcept
{
    subtract1Scalar(value, output, size);
}

template <>
void subtract1<float>(float value, float* output, unsigned size) noexcept;

template <class T>
void subtract1(T value, absl::Span<T> output) noexcept
{
    subtract1<T>(value, output.data(), output.size());
}

/**
 * @brief Copy a span in another
 *
 * @tparam T the underlying type
 * @param input
 * @param output
 * @param size
 */
template <class T>
void copy(const T* input, T* output, unsigned size) noexcept
{
    std::copy(input, input + size, output);
}

template <>
void copy<float>(const float* input, float* output, unsigned size) noexcept;

template <class T>
void copy(absl::Span<const T> input, absl::Span<T> output) noexcept
{
    CHECK_SPAN_SIZES(input, output);
    copy<T>(input.data(), output.data(), minSpanSize(input, output));
}


/**
 * @brief Computes the mean of a span
 *
 * @tparam T the underlying type
 * @param vector
 * @param size
 * @return T
 */
template <class T>
T mean(const T* vector, unsigned size) noexcept
{
    meanScalar(vector, size);
}

template <>
float mean<float>(const float* vector, unsigned size) noexcept;

template <class T>
T mean(absl::Span<const T> vector) noexcept
{
    return mean(vector.data(), vector.size());
}

/**
 * @brief Computes the sum of squares of a span
 *
 * @tparam T the underlying type
 * @tparam SIMD use the SIMD version or the scalar version
 * @param vector
 * @return T
 */
template <class T>
T sumSquares(const T* vector, unsigned size) noexcept
{
    return sumSquaresScalar(vector, size);
}

template <>
float sumSquares<float>(const float* vector, unsigned size) noexcept;

template <class T>
T sumSquares(absl::Span<const T> vector) noexcept
{
    return sumSquares(vector.data(), vector.size());
}

/**
 * @brief Computes the mean squared of a span
 *
 * @tparam T the underlying type
 * @param vector
 * @return T
 */
template <class T>
T meanSquared(const T* vector, unsigned size) noexcept
{
    T sum = sumSquares(vector, size);
    return sum / size;
}

template <class T>
T meanSquared(absl::Span<const T> vector) noexcept
{
    return meanSquared(vector.data(), vector.size());
}

/**
 * @brief Computes the cumulative sum of a span.
 * The first output is the same as the first input.
 *
 * @tparam T the underlying type
 * @param input
 * @param output
 * @param size
 */
template <class T>
void cumsum(const T* input, T* output, unsigned size) noexcept
{
    cumsumScalar(input, output, size);
}

template <>
void cumsum<float>(const float* input, float* output, unsigned size) noexcept;

template <class T>
void cumsum(absl::Span<const T> input, absl::Span<T> output) noexcept
{
    CHECK_SPAN_SIZES(input, output);
    cumsum<T>(input.data(), output.data(), minSpanSize(input, output));
}

// FIXME: This should go away once the changes from the resampler are in

namespace _internals {
    template <class T>
    void snippetSFZInterpolationCast(const T*& floatJump, int*& jump, T*& coeff)
    {
        constexpr float maxJump { 1 << 24 };
        const float limitedJump = min(maxJump, *floatJump);
        *jump = static_cast<int>(limitedJump);
        *coeff = limitedJump - static_cast<float>(*jump);
        incrementAll(floatJump, coeff, jump);
    }
}

/**
 * @brief Computes the linear interpolation coefficients for a floating point index
 * and extracts the integer index of the elements to interpolate
 *
 * @tparam T the underlying type
 * @param floatJumps the floating point indices
 * @param jumps the integer indices outputs
 * @param leftCoeffs the left interpolation coefficients
 * @param rightCoeffs the right interpolation coefficients
 */
template <class T>
void sfzInterpolationCast(absl::Span<const T> floatJumps, absl::Span<int> jumps, absl::Span<T> coeffs) noexcept
{
    SFIZZ_CHECK(jumps.size() >= floatJumps.size());
    SFIZZ_CHECK(jumps.size() == coeffs.size());

    auto* floatJump = floatJumps.data();
    auto* jump = jumps.data();
    auto* coeff = coeffs.data();
    const auto* sentinel = floatJump + min(floatJumps.size(), jumps.size(), coeffs.size());

    while (floatJump < sentinel)
        _internals::snippetSFZInterpolationCast(floatJump, jump, coeff);
}

/**
 * @brief Computes the differential of a span (successive differences).
 * The first output is the same as the first input.
 *
 * @tparam T the underlying type
 * @param input
 * @param output
 * @param size
 */
template <class T>
void diff(const T* input, T* output, unsigned size) noexcept
{
    diffScalar(input, output, size);
}

template <>
void diff<float>(const float* input, float* output, unsigned size) noexcept;

template <class T>
void diff(absl::Span<const T> input, absl::Span<T> output) noexcept
{
    CHECK_SPAN_SIZES(input, output);
    diff<T>(input.data(), output.data(), minSpanSize(input, output));
}

/**
 * @brief Clamp a vector between a low and high bound
 *
 * @tparam T the underlying type
 * @param input
 * @param low
 * @param high
 * @param size
 */
template <class T>
void clampAll(T* input, T low, T high, unsigned size) noexcept
{
    clampAllScalar(input, low, high, size);
}

template <>
void clampAll<float>(float* input, float low, float high, unsigned size) noexcept;

template <class T>
void clampAll(absl::Span<T> input, T low, T high) noexcept
{
    clampAll<T>(input.data(), low, high, input.size());
}

template <class T>
void clampAll(absl::Span<T> input, sfz::Range<T> range) noexcept
{
    clampAll<T>(input.data(), range.getStart(), range.getEnd(), input.size());
}

/**
 * @brief Check that all values are within bounds (inclusive)
 *
 * @tparam T the underlying type
 * @param input
 * @param low
 * @param high
 * @param size
 */
template <class T>
bool allWithin(const T* input, T low, T high, unsigned size) noexcept
{
    return allWithinScalar(input, low, high, size);
}

template <>
bool allWithin<float>(const float* input, float low, float high, unsigned size) noexcept;

template <class T>
bool allWithin(absl::Span<const T> input, T low, T high) noexcept
{
    return allWithin<T>(input.data(), low, high, input.size());
}

} // namespace sfz
