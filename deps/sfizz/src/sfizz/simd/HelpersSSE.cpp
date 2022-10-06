// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "HelpersSSE.h"
#include "../SIMDConfig.h"
#include "../MathHelpers.h"
#include "Common.h"
#include <array>

#if SFIZZ_HAVE_SSE2
#include <immintrin.h>
using Type = float;
constexpr unsigned TypeAlignment = 4;
constexpr unsigned ByteAlignment = TypeAlignment * sizeof(Type);
#endif

void readInterleavedSSE(const float* input, float* outputLeft, float* outputRight, unsigned inputSize) noexcept
{
    const auto* sentinel = input + inputSize - 1;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(input + inputSize - TypeAlignment);
    while (unaligned<ByteAlignment>(input, outputLeft, outputRight) && input < lastAligned) {
        *outputLeft++ = *input++;
        *outputRight++ = *input++;
    }

    while (input < lastAligned) {
        auto register0 = _mm_load_ps(input);
        auto register1 = _mm_load_ps(input + TypeAlignment);
        auto register2 = register0;
        // register 2 holds the copy of register 0 that is going to get erased by the first operation
        // Remember that the bit mask reads from the end; 10 00 10 00 means
        // "take 0 from a, take 2 from a, take 0 from b, take 2 from b"
        register0 = _mm_shuffle_ps(register0, register1, 0b10001000);
        register1 = _mm_shuffle_ps(register2, register1, 0b11011101);
        _mm_store_ps(outputLeft, register0);
        _mm_store_ps(outputRight, register1);
        incrementAll<TypeAlignment>(input, input, outputLeft, outputRight);
    }

    // NEON wip
    // auto reg = vld2q_f32(in);
    // vst1q_f32(lOut, reg.val[0]);
    // vst1q_f32(rOut, reg.val[1]);
#endif

    while (input < sentinel) {
        *outputLeft++ = *input++;
        *outputRight++ = *input++;
    }
}

void writeInterleavedSSE(const float* inputLeft, const float* inputRight, float* output, unsigned outputSize) noexcept
{
    const auto* sentinel = output + outputSize - 1;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(output + outputSize - TypeAlignment);
    while (unaligned<ByteAlignment>(output, inputRight, inputLeft) && output < lastAligned) {
        *output++ = *inputLeft++;
        *output++ = *inputRight++;
    }

    while (output < lastAligned) {
        const auto lInRegister = _mm_load_ps(inputLeft);
        const auto rInRegister = _mm_load_ps(inputRight);
        const auto outRegister1 = _mm_unpacklo_ps(lInRegister, rInRegister);
        _mm_store_ps(output, outRegister1);
        const auto outRegister2 = _mm_unpackhi_ps(lInRegister, rInRegister);
        _mm_store_ps(output + 4, outRegister2);
        incrementAll<TypeAlignment>(output, output, inputLeft, inputRight);
    }
#endif

    while (output < sentinel) {
        *output++ = *inputLeft++;
        *output++ = *inputRight++;
    }
}

void gain1SSE(float gain, const float* input, float* output, unsigned size) noexcept
{
    const auto* sentinel = output + size;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    const auto mmGain = _mm_set1_ps(gain);
    while (unaligned<ByteAlignment>(input, output) && output < lastAligned)
        *output++ = gain * (*input++);

    while (output < lastAligned) {
        _mm_store_ps(output, _mm_mul_ps(mmGain, _mm_load_ps(input)));
        incrementAll<TypeAlignment>(input, output);
    }
#endif

    while (output < sentinel)
        *output++ = gain * (*input++);
}

void gainSSE(const float* gain, const float* input, float* output, unsigned size) noexcept
{
    const auto* sentinel = output + size;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(input, output) && output < lastAligned)
        *output++ = (*gain++) * (*input++);

    while (output < lastAligned) {
        _mm_store_ps(output, _mm_mul_ps(_mm_load_ps(gain), _mm_load_ps(input)));
        incrementAll<TypeAlignment>(gain, input, output);
    }
#endif

    while (output < sentinel)
        *output++ = (*gain++) * (*input++);
}

void divideSSE(const float* input, const float* divisor, float* output, unsigned size) noexcept
{
    const auto* sentinel = output + size;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(input, output) && output < lastAligned)
        *output++ = (*input++) / (*divisor++);

    while (output < lastAligned) {
        _mm_store_ps(output, _mm_div_ps(_mm_load_ps(input), _mm_load_ps(divisor)));
        incrementAll<TypeAlignment>(divisor, input, output);
    }
#endif

    while (output < sentinel)
        *output++ = (*input++) / (*divisor++);
}

void multiplyAddSSE(const float* gain, const float* input, float* output, unsigned size) noexcept
{
    const auto* sentinel = output + size;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(input, output) && output < lastAligned)
        *output++ += (*gain++) * (*input++);

    while (output < lastAligned) {
        auto mmOut = _mm_load_ps(output);
        mmOut = _mm_add_ps(_mm_mul_ps(_mm_load_ps(gain), _mm_load_ps(input)), mmOut);
        _mm_store_ps(output, mmOut);
        incrementAll<TypeAlignment>(gain, input, output);
    }
#endif

    while (output < sentinel)
        *output++ += (*gain++) * (*input++);
}

void multiplyAdd1SSE(float gain, const float* input, float* output, unsigned size) noexcept
{
    const auto* sentinel = output + size;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(input, output) && output < lastAligned)
        *output++ += gain * (*input++);

    auto mmGain = _mm_set1_ps(gain);
    while (output < lastAligned) {
        auto mmOut = _mm_load_ps(output);
        mmOut = _mm_add_ps(_mm_mul_ps(mmGain, _mm_load_ps(input)), mmOut);
        _mm_store_ps(output, mmOut);
        incrementAll<TypeAlignment>(input, output);
    }
#endif

    while (output < sentinel)
        *output++ += gain * (*input++);
}

void multiplyMulSSE(const float* gain, const float* input, float* output, unsigned size) noexcept
{
    const auto* sentinel = output + size;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(input, output) && output < lastAligned)
        *output++ *= (*gain++) * (*input++);

    while (output < lastAligned) {
        auto mmOut = _mm_load_ps(output);
        mmOut = _mm_mul_ps(_mm_mul_ps(_mm_load_ps(gain), _mm_load_ps(input)), mmOut);
        _mm_store_ps(output, mmOut);
        incrementAll<TypeAlignment>(gain, input, output);
    }
#endif

    while (output < sentinel)
        *output++ *= (*gain++) * (*input++);
}

void multiplyMul1SSE(float gain, const float* input, float* output, unsigned size) noexcept
{
    const auto* sentinel = output + size;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(input, output) && output < lastAligned)
        *output++ *= gain * (*input++);

    auto mmGain = _mm_set1_ps(gain);
    while (output < lastAligned) {
        auto mmOut = _mm_load_ps(output);
        mmOut = _mm_mul_ps(_mm_mul_ps(mmGain, _mm_load_ps(input)), mmOut);
        _mm_store_ps(output, mmOut);
        incrementAll<TypeAlignment>(input, output);
    }
#endif

    while (output < sentinel)
        *output++ *= gain * (*input++);
}

float linearRampSSE(float* output, float start, float step, unsigned size) noexcept
{
    const auto* sentinel = output + size;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(output) && output < lastAligned) {
        *output++ = start;
        start += step;
    }

    auto mmStart = _mm_set1_ps(start - step);
    auto mmStep = _mm_set_ps(step + step + step + step, step + step + step, step + step, step);
    while (output < lastAligned) {
        mmStart = _mm_add_ps(mmStart, mmStep);
        _mm_store_ps(output, mmStart);
        mmStart = _mm_shuffle_ps(mmStart, mmStart, _MM_SHUFFLE(3, 3, 3, 3));
        incrementAll<TypeAlignment>(output);
    }
    start = _mm_cvtss_f32(mmStart) + step;
#endif

    while (output < sentinel) {
        *output++ = start;
        start += step;
    }
    return start;
}

float multiplicativeRampSSE(float* output, float start, float step, unsigned size) noexcept
{
    const auto* sentinel = output + size;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(output) && output < lastAligned) {
        *output++ = start;
        start *= step;
    }

    auto mmStart = _mm_set1_ps(start / step);
    auto mmStep = _mm_set_ps(step * step * step * step, step * step * step, step * step, step);
    while (output < lastAligned) {
        mmStart = _mm_mul_ps(mmStart, mmStep);
        _mm_store_ps(output, mmStart);
        mmStart = _mm_shuffle_ps(mmStart, mmStart, _MM_SHUFFLE(3, 3, 3, 3));
        incrementAll<TypeAlignment>(output);
    }
    start = _mm_cvtss_f32(mmStart) * step;
#endif

    while (output < sentinel) {
        *output++ = start;
        start *= step;
    }
    return start;
}

void addSSE(const float* input, float* output, unsigned size) noexcept
{
    const auto* sentinel = output + size;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(input, output) && output < lastAligned)
        *output++ += *input++;

    while (output < lastAligned) {
        _mm_store_ps(output, _mm_add_ps(_mm_load_ps(output), _mm_load_ps(input)));
        incrementAll<TypeAlignment>(input, output);
    }
#endif

    while (output < sentinel)
        *output++ += *input++;
}

void add1SSE(float value, float* output, unsigned size) noexcept
{
    const auto* sentinel = output + size;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(output) && output < lastAligned)
        *output++ += value;

    const auto mmValue = _mm_set1_ps(value);
    while (output < lastAligned) {
        _mm_store_ps(output, _mm_add_ps(_mm_load_ps(output), mmValue));
        incrementAll<TypeAlignment>(output);
    }
#endif

    while (output < sentinel)
        *output++ += value;
}

void subtractSSE(const float* input, float* output, unsigned size) noexcept
{
    const auto* sentinel = output + size;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(input, output) && output < lastAligned)
        *output++ -= *input++;

    while (output < lastAligned) {
        _mm_store_ps(output, _mm_sub_ps(_mm_load_ps(output), _mm_load_ps(input)));
        incrementAll<TypeAlignment>(input, output);
    }
#endif

    while (output < sentinel)
        *output++ -= *input++;
}

void subtract1SSE(float value, float* output, unsigned size) noexcept
{
    const auto* sentinel = output + size;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(output) && output < lastAligned)
        *output++ -= value;

    const auto mmValue = _mm_set1_ps(value);
    while (output < lastAligned) {
        _mm_store_ps(output, _mm_sub_ps(_mm_load_ps(output), mmValue));
        incrementAll<TypeAlignment>(output);
    }
#endif

    while (output < sentinel)
        *output++ -= value;
}

void copySSE(const float* input, float* output, unsigned size) noexcept
{
    // The sentinel is the input here
    const auto* sentinel = input + size;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(input, output) && input < lastAligned)
        *output++ = *input++;

    while (input < lastAligned) {
        _mm_store_ps(output, _mm_load_ps(input));
        incrementAll<TypeAlignment>(input, output);
    }
#endif

    std::copy(input, sentinel, output);
}

float meanSSE(const float* vector, unsigned size) noexcept
{
    const auto* sentinel = vector + size;

    float result { 0.0f };
    if (size == 0)
        return result;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(vector) && vector < lastAligned)
        result += *vector++;

    auto mmSums = _mm_setzero_ps();
    while (vector < lastAligned) {
        mmSums = _mm_add_ps(mmSums, _mm_load_ps(vector));
        incrementAll<TypeAlignment>(vector);
    }

    std::array<float, 4> sseResult;
    _mm_store_ps(sseResult.data(), mmSums);

    for (auto sseValue : sseResult)
        result += sseValue;
#endif

    while (vector < sentinel)
        result += *vector++;

    return result / static_cast<float>(size);
}

float sumSquaresSSE(const float* vector, unsigned size) noexcept
{
    const auto* sentinel = vector + size;

    float result { 0.0f };
    if (size == 0)
        return result;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(vector) && vector < lastAligned) {
        result += (*vector) * (*vector);
        vector++;
    }

    auto mmSums = _mm_setzero_ps();
    while (vector < lastAligned) {
        const auto mmValues = _mm_load_ps(vector);
        mmSums = _mm_add_ps(mmSums, _mm_mul_ps(mmValues, mmValues));
        incrementAll<TypeAlignment>(vector);
    }

    std::array<float, 4> sseResult;
    _mm_store_ps(sseResult.data(), mmSums);

    for (auto sseValue : sseResult)
        result += sseValue;
#endif

    while (vector < sentinel) {
        result += (*vector) * (*vector);
        vector++;
    }

    return result;
}

void cumsumSSE(const float* input, float* output, unsigned size) noexcept
{
    if (size == 0)
        return;

    const auto* sentinel = output + size;
    *output++ = *input++;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(input, output) && output < lastAligned) {
        *output = *(output - 1) + *input;
        incrementAll(input, output);
    }

    auto mmOutput = _mm_set_ps1(*(output - 1));
    while (output < lastAligned) {
        auto mmOffset = _mm_load_ps(input);
        mmOffset = _mm_add_ps(mmOffset, _mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(mmOffset), 4)));
        mmOffset = _mm_add_ps(mmOffset, _mm_shuffle_ps(_mm_setzero_ps(), mmOffset, _MM_SHUFFLE(1, 0, 0, 0)));
        mmOutput = _mm_add_ps(mmOutput, mmOffset);
        _mm_store_ps(output, mmOutput);
        mmOutput = _mm_shuffle_ps(mmOutput, mmOutput, _MM_SHUFFLE(3, 3, 3, 3));
        incrementAll<TypeAlignment>(input, output);
    }
#endif

    while (output < sentinel) {
        *output = *(output - 1) + *input;
        incrementAll(input, output);
    }
}

void diffSSE(const float* input, float* output, unsigned size) noexcept
{
    if (size == 0)
        return;

    const auto* sentinel = output + size;
    *output++ = *input++;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(input, output) && output < lastAligned) {
        *output = *input - *(input - 1);
        incrementAll(input, output);
    }

    auto mmBase = _mm_set_ps1(*(input - 1));
    while (output < lastAligned) {
        auto mmOutput = _mm_load_ps(input);
        auto mmNextBase = _mm_shuffle_ps(mmOutput, mmOutput, _MM_SHUFFLE(3, 3, 3, 3));
        mmOutput = _mm_sub_ps(mmOutput, mmBase);
        mmBase = mmNextBase;
        mmOutput = _mm_sub_ps(mmOutput, _mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(mmOutput), 4)));
        _mm_store_ps(output, mmOutput);
        incrementAll<TypeAlignment>(input, output);
    }
#endif

    while (output < sentinel) {
        *output = *input - *(input - 1);
        incrementAll(input, output);
    }
}

void clampAllSSE(float* input, float low, float high, unsigned size) noexcept
{
    if (size == 0)
        return;

    const auto* sentinel = input + size;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(input) && input < lastAligned){
        const float clampedAbove = *input > high ? high : *input;
        *input = clampedAbove < low ? low : clampedAbove;
        incrementAll(input);
    }

    const auto mmLow = _mm_set1_ps(low);
    const auto mmHigh = _mm_set1_ps(high);
    while (input < lastAligned) {
        const auto mmIn = _mm_load_ps(input);
        _mm_store_ps(input, _mm_max_ps(_mm_min_ps(mmIn, mmHigh), mmLow));
        incrementAll<TypeAlignment>(input);
    }
#endif

    while (input < sentinel) {
        const float clampedAbove = *input > high ? high : *input;
        *input = clampedAbove < low ? low : clampedAbove;
        incrementAll(input);
    }
}

bool allWithinSSE(const float* input, float low, float high, unsigned size) noexcept
{
    if (size == 0)
        return true;

    if (low > high)
        std::swap(low, high);

    const auto* sentinel = input + size;

#if SFIZZ_HAVE_SSE2
    const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
    while (unaligned<ByteAlignment>(input) && input < lastAligned){
        if (*input < low || *input > high)
            return false;

        incrementAll(input);
    }

    const auto mmLow = _mm_set1_ps(low);
    const auto mmHigh = _mm_set1_ps(high);
    while (input < lastAligned) {
        const auto mmIn = _mm_load_ps(input);
        const auto mmOutside = _mm_or_ps(_mm_cmplt_ps(mmIn, mmLow), _mm_cmpgt_ps(mmIn, mmHigh));
        if (_mm_movemask_ps(mmOutside) != 0)
            return false;

        incrementAll<TypeAlignment>(input);
    }
#endif

    while (input < sentinel) {
        if (*input < low || *input > high)
            return false;

        incrementAll(input);
    }

    return true;
}
