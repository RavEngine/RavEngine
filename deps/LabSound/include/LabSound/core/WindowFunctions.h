// License: BSD 2 Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#pragma once

#ifndef lab_window_functions_h
#define lab_window_functions_h

#include <cmath>
#include <vector>

#include "LabSound/core/Macros.h"

namespace lab
{
enum class WindowFunction
{
    rectangle,  // aka the boxcar or Dirichlet window
    cosine,  // aka the sine window
    hann,  // generalized raised cosine, order 1, aka the raised cosine window
    hamming,  // generalized raised cosine, order 1 (modified hann)
    blackman,  // generalized raised cosine, order 2
    nutall,  // generalized raised cosine, order 3
    blackman_harris,  // generalized raised cosine, order 3
    blackman_nutall,  // generalized raised cosine, order 3
    hann_poisson,  // Hann window multiplied by a Poisson window
    gaussian50,  // gaussian with a sigma of 0.50
    gaussian25,  // gaussian with a sigma of 0.25
    welch,  //
    bartlett,  // aka the (symmetric) triangular window
    bartlett_hann,  //
    parzen,  // B-spline, order 4 (a triangle shape)
    flat_top,  // generalized raised cosine, order 4
    lanczos  // aka the sinc window
};

static constexpr char const * const s_window_types[] = {
    "rectangle",
    "cosine",
    "hann",
    "hamming",
    "blackman",
    "nutall",
    "blackman_harris",
    "blackman_nutall",
    "hann_poisson",
    "gaussian50",
    "gaussian25",
    "welch",
    "bartlett",
    "bartlett_hann",
    "parzen",
    "flat_top",
    "lanczos",
    nullptr
};

// Inspired by https://github.com/idiap/libssp/blob/master/ssp/window.cpp
// These are implementations of the generalized raised cosine window, up to order 4
// https://ccrma.stanford.edu/~jos/sasp/Hann_Hanning_Raised_Cosine.html
namespace detail
{
    inline float sinc(const float x)
    {
        return (x == 0.0f) ? 1.0f : std::sin(x * static_cast<float>(LAB_PI)) / (x * static_cast<float>(LAB_PI));
    }

    inline float gen_cosine_1(const float max_index, const uint32_t idx, const float alpha, const float beta)
    {
        return alpha - beta * std::cos((static_cast<float>(LAB_TAU) * idx) / max_index);
    }

    inline float gen_cosine_2(const float max_index, const uint32_t idx, const float alpha, const float beta, const float gamma)
    {
        return gen_cosine_1(max_index, idx, alpha, beta) + gamma * std::cos((2 * static_cast<float>(LAB_TAU) * idx) / max_index);
    }

    inline float gen_cosine_3(const float max_index, const uint32_t idx, const float alpha, const float beta, const float gamma, const float delta)
    {
        return gen_cosine_2(max_index, idx, alpha, beta, gamma) - delta * std::cos((3 * static_cast<float>(LAB_TAU) * idx) / max_index);
    }

    inline float gen_cosine_4(const float max_index, const uint32_t idx, const float alpha, const float beta, const float gamma, const float delta, const float epsilon)
    {
        return gen_cosine_3(max_index, idx, alpha, beta, gamma, delta) + epsilon * std::cos((4 * static_cast<float>(LAB_TAU) * idx) / max_index);
    }

    inline float gaussian(const float max_index, const uint32_t idx, const float sigma)
    {
        const float fract = (idx - (max_index * 0.5f)) / (sigma * (max_index * 0.5f));
        return std::exp(-0.5f * (fract * fract));
    }
}

// Reference https://github.com/spurious/snd-mirror/blob/master/clm.c
inline void ApplyWindowFunctionInplace(const WindowFunction type, float * buffer, const int window_size)
{
    const float max_index = static_cast<float>(window_size) - 1.f;

    switch (type)
    {
        case WindowFunction::rectangle:
        {
            for (int i = 0; i < window_size; ++i)
            {
                buffer[i] = 1.f;  // or * 0.5f?
            }
        }
        break;

        case WindowFunction::cosine:
        {
            for (int i = 0; i < window_size; ++i)
            {
                buffer[i] *= std::sin((static_cast<float>(LAB_PI) * i) / max_index);
            }
        }
        break;

        case WindowFunction::hann:
        {
            for (int i = 0; i < window_size; ++i)
            {
                buffer[i] *= detail::gen_cosine_1(max_index, i, 0.5f, 0.5f);
            }
        }
        break;

        case WindowFunction::hamming:
        {
            for (int i = 0; i < window_size; ++i)
            {
                buffer[i] *= detail::gen_cosine_1(max_index, i, 0.54f, 0.46f);
            }
        }
        break;

        case WindowFunction::blackman:
        {
            for (int i = 0; i < window_size; ++i)
            {
                buffer[i] *= detail::gen_cosine_2(max_index, i, 0.42f, 0.50f, 0.08f);
            }
        }
        break;

        case WindowFunction::nutall:
        {
            for (int i = 0; i < window_size; ++i)
            {
                buffer[i] *= detail::gen_cosine_3(max_index, i, 0.355768f, 0.487396f, 0.144232f, 0.012604f);
            }
        }
        break;

        case WindowFunction::blackman_harris:
        {
            for (int i = 0; i < window_size; ++i)
            {
                buffer[i] *= detail::gen_cosine_3(max_index, i, 0.35875f, 0.48829f, 0.14128f, 0.01168f);
            }
        }
        break;

        case WindowFunction::blackman_nutall:
        {
            for (int i = 0; i < window_size; ++i)
            {
                buffer[i] *= detail::gen_cosine_3(max_index, i, 0.3635819f, 0.4891775f, 0.1365995f, 0.0106411f);
            }
        }
        break;

        case WindowFunction::hann_poisson:
        {
            for (int i = 0; i < window_size; ++i)
            {
                const float alpha = 2.f;
                const float a = 1.f - std::cos((static_cast<float>(LAB_TAU) * i) / max_index);
                const float b = (-alpha * std::abs(max_index - 2.f * i)) / max_index;
                buffer[i] *= 0.5f * a * exp(b);
            }
        }
        break;

        case WindowFunction::gaussian50:
        {
            for (int i = 0; i < window_size; ++i)
            {
                buffer[i] *= detail::gaussian(max_index, i, 0.50f);
            }
        }
        break;

        case WindowFunction::gaussian25:
        {
            for (int i = 0; i < window_size; ++i)
            {
                buffer[i] *= detail::gaussian(max_index, i, 0.25f);
            }
        }
        break;

        case WindowFunction::welch:
        {
            for (int i = 0; i < window_size; ++i)
            {
                const float num = i - (max_index * 0.5f);
                const float denom = (window_size + 1.f) * 0.5f;
                const float fract = num / denom;
                buffer[i] *= 1.f - fract * fract;
            }
        }
        break;

        case WindowFunction::bartlett:
        {
            for (int i = 0; i < window_size; ++i)
            {
                buffer[i] *= 2.f / (window_size - 1.f) * (max_index / 2.f - std::abs(i - max_index / 2.f));
            }
        }
        break;

        case WindowFunction::bartlett_hann:
        {
            for (int i = 0; i < window_size; ++i)
            {
                buffer[i] *= detail::gen_cosine_2(max_index, i, 0.63f, 0.48f, 0.38f);
            }
        }
        break;

        case WindowFunction::parzen:
        {
            for (int i = 0; i < window_size; ++i)
            {
                buffer[i] *= 1.f - abs((2.f * i - window_size) / (window_size + 1.f));
            }
        }
        break;

        case WindowFunction::flat_top:
        {
            for (int i = 0; i < window_size; ++i)
            {
                buffer[i] *= detail::gen_cosine_4(max_index, i, 1.f, 1.93f, 1.29f, 0.388f, 0.028f);
            }
        }
        break;

        case WindowFunction::lanczos:
        {
            for (int i = 0; i < window_size; ++i)
            {
                buffer[i] *= detail::sinc(2.f * i / max_index - 1.f);
            }
        }
        break;
    }
}

}  // namespace lab

#endif  // end lab_window_functions_h
