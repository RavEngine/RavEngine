// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

/**
 * @file MathHelpers.h
 * @brief Contains math helper functions and math constants
 */
#pragma once
#include "Config.h"
#include "SIMDConfig.h"
#include "utility/Debug.h"
#include "utility/Macros.h"
#include "absl/types/span.h"
#include <algorithm>
#include <array>
#include <random>
#include <limits>
#include <type_traits>
#include <cmath>
#include <cfenv>
#include <simde/simde-features.h>
#if SIMDE_NATURAL_VECTOR_SIZE_GE(128)
#include <simde/x86/sse.h>
#endif

#if __cplusplus >= 201703L && defined(__cpp_lib_math_special_functions)
static double i0(double x) { return std::cyl_bessel_i(0.0, x); }
#else
// external Bessel function from cephes
extern "C" double i0(double x);
#endif

template <class T>
constexpr T max(T op1, T op2)
{
    return op1 > op2 ? op1 : op2;
}

template <class T, class... Args>
constexpr T max(T op1, Args... rest)
{
    return max(op1, max(rest...));
}

template <class T>
constexpr T min(T op1, T op2)
{
    return op1 > op2 ? op2 : op1;
}

template <class T, class... Args>
constexpr T min(T op1, Args... rest)
{
    return min(op1, min(rest...));
}

/**
 * @brief Compute the square of the value
 *
 * @param op
 * @return T
 */
template <class T>
constexpr T power2(T in)
{
    return in * in;
}

/**
 * @brief Converts db values into power (applies 10**(in/10))
 *
 * @tparam Type
 * @param in
 * @return Type
 */
template <class Type>
constexpr Type db2pow(Type in)
{
    return std::pow(static_cast<Type>(10.0), in * static_cast<Type>(0.1));
}

/**
 * @brief Converts power values into dB (applies 10log10(in))
 *
 * @tparam Type
 * @param in
 * @return Type
 */
template <class Type>
constexpr Type pow2db(Type in)
{
    return static_cast<Type>(10.0) * std::log10(in);
}

/**
 * @brief Converts dB values to magnitude (applies 10**(in/20))
 *
 * @tparam Type
 * @param in
 * @return constexpr Type
 */
template <class Type>
constexpr Type db2mag(Type in)
{
    return std::pow(static_cast<Type>(10.0), in * static_cast<Type>(0.05));
}

/**
 * @brief Converts magnitude values into dB (applies 20log10(in))
 *
 * @tparam Type
 * @param in
 * @return Type
 */
template <class Type>
constexpr Type mag2db(Type in)
{
    return static_cast<Type>(20.0) * std::log10(in);
}

/**
 * @brief Converts a midi note to a frequency value
 *
 * @param noteNumber
 * @return float
 */
inline float midiNoteFrequency(const int noteNumber)
{
    return 440.0f * std::pow(2.0f, (noteNumber - 69) * (1.0f / 12.0f));
}

/**
 * @brief Clamps a value between bounds, including the bounds!
 *
 * @tparam T
 * @param v
 * @param lo
 * @param hi
 * @return T
 */
template <class T>
constexpr T clamp(T v, T lo, T hi)
{
    return max(min(v, hi), lo);
}

/**
 * @brief Compute the floating-point remainder (fmod)
 *
 * @tparam T
 * @param x
 * @param m
 * @return T
 */
template <class T>
inline constexpr T fastFmod(T x, T m)
{
    return x - m * static_cast<int>(x / m);
}

template <int Increment = 1, class T>
inline CXX14_CONSTEXPR void incrementAll(T& only)
{
    only += Increment;
}

template <int Increment = 1, class T, class... Args>
inline CXX14_CONSTEXPR void incrementAll(T& first, Args&... rest)
{
    first += Increment;
    incrementAll<Increment>(rest...);
}

/**
 * @brief Compute the 3rd-order Hermite interpolation polynomial.
 *
 * @tparam R
 * @param x
 * @return R
 */
template <class R>
R hermite3(R x)
{
    x = std::abs(x);
    R x2 = x * x;
    R x3 = x2 * x;
    R y = 0;
    R q = R(5./2.) * x2; // a reoccurring term
    R p1 = R(1) - q + R(3./2.) * x3;
    R p2 = R(2) - R(4) * x + q - R(1./2.) * x3;
    y = (x < R(2)) ? p2 : y;
    y = (x < R(1)) ? p1 : y;
    return y;
}

#if SIMDE_NATURAL_VECTOR_SIZE_GE(128)
/**
 * @brief Compute 4 parallel elements of the 3rd-order Hermite interpolation polynomial.
 *
 * @param x
 * @return simde__m128
 */
inline simde__m128 hermite3x4(simde__m128 x)
{
    x = simde_x_mm_abs_ps(x);
    simde__m128 x2 = simde_mm_mul_ps(x, x);
    simde__m128 x3 = simde_mm_mul_ps(x2, x);
    simde__m128 y = simde_mm_set1_ps(0.0f);
    simde__m128 q = simde_mm_mul_ps(simde_mm_set1_ps(5./2.), x2);
    simde__m128 p1 = simde_mm_add_ps(simde_mm_sub_ps(simde_mm_set1_ps(1), q), simde_mm_mul_ps(simde_mm_set1_ps(3./2.), x3));
    simde__m128 p2 = simde_mm_sub_ps(simde_mm_add_ps(simde_mm_sub_ps(simde_mm_set1_ps(2), simde_mm_mul_ps(simde_mm_set1_ps(4), x)), q), simde_mm_mul_ps(simde_mm_set1_ps(1./2.), x3));
    simde__m128 m2 = simde_mm_cmple_ps(x, simde_mm_set1_ps(2));
    y = simde_mm_or_ps(simde_mm_and_ps(m2, p2), simde_mm_andnot_ps(m2, y));
    simde__m128 m1 = simde_mm_cmple_ps(x, simde_mm_set1_ps(1));
    y = simde_mm_or_ps(simde_mm_and_ps(m1, p1), simde_mm_andnot_ps(m1, y));
    return y;
}
#endif

/**
 * @brief Compute the 3rd-order B-spline interpolation polynomial.
 *
 * @tparam R
 * @param x
 * @return R
 */
template <class R>
R bspline3(R x)
{
    x = std::abs(x);
    R x2 = x * x;
    R x3 = x2 * x;
    R y = 0;
    R p1 = R(2./3.) - x2 + R(1./2.) * x3;
    R p2 = R(4./3.) - R(2) * x + x2 - R(1./6.) * x3;
    y = (x < R(2)) ? p2 : y;
    y = (x < R(1)) ? p1 : y;
    return y;
}

#if SIMDE_NATURAL_VECTOR_SIZE_GE(128)
/**
 * @brief Compute 4 parallel elements of the 3rd-order B-spline interpolation polynomial.
 *
 * @param x
 * @return simde__m128
 */
inline simde__m128 bspline3x4(simde__m128 x)
{
    x = simde_x_mm_abs_ps(x);
    simde__m128 x2 = simde_mm_mul_ps(x, x);
    simde__m128 x3 = simde_mm_mul_ps(x2, x);
    simde__m128 y = simde_mm_set1_ps(0.0f);
    simde__m128 p1 = simde_mm_add_ps(simde_mm_sub_ps(simde_mm_set1_ps(2./3.), x2), simde_mm_mul_ps(simde_mm_set1_ps(1./2.), x3));
    simde__m128 p2 = simde_mm_sub_ps(simde_mm_add_ps(simde_mm_sub_ps(simde_mm_set1_ps(4./3.), simde_mm_mul_ps(simde_mm_set1_ps(2), x)), x2), simde_mm_mul_ps(simde_mm_set1_ps(1./6.), x3));
    simde__m128 m2 = simde_mm_cmple_ps(x, simde_mm_set1_ps(2));
    y = simde_mm_or_ps(simde_mm_and_ps(m2, p2), simde_mm_andnot_ps(m2, y));
    simde__m128 m1 = simde_mm_cmple_ps(x, simde_mm_set1_ps(1));
    y = simde_mm_or_ps(simde_mm_and_ps(m1, p1), simde_mm_andnot_ps(m1, y));
    return y;
}
#endif

template <class Type>
constexpr Type pi() { return static_cast<Type>(3.141592653589793238462643383279502884); };
template <class Type>
constexpr Type twoPi() { return pi<Type>() * 2; };
template <class Type>
constexpr Type piTwo() { return pi<Type>() / 2; };
template <class Type>
constexpr Type piFour() { return pi<Type>() / 4; };
template <class Type>
constexpr Type sqrtTwo() { return static_cast<Type>(1.414213562373095048801688724209698078569671875376948073176); };
template <class Type>
constexpr Type sqrtTwoInv() { return static_cast<Type>(0.707106781186547524400844362104849039284835937688474036588); };

constexpr unsigned int mask(int x)
{
    return (1U << x) - 1;
}

/**
 * @brief lround for positive values
 * This optimizes a bit better by ignoring the negative code path
 *
 * @tparam T
 * @param value
 * @return constexpr long int
 */
template<class T, absl::enable_if_t<std::is_floating_point<T>::value, int> = 0 >
constexpr long int lroundPositive(T value)
{
    return static_cast<int>(0.5f + value); // NOLINT
}

/**
 * @brief Compute the next power of 2
 *
 * @param v An integer
 * @return The next power of 2, inclusive from @p v
 */
inline uint32_t nextPow2(uint32_t v)
{
    // Bit Twiddling Hacks
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

/**
   @brief Wrap a normalized phase into the domain [0;1[
 */
template <class T>
static T wrapPhase(T phase)
{
    T wrapped = phase - static_cast<int>(phase);
    wrapped += wrapped < 0;
    return wrapped;
}

/**
   @brief A fraction which is parameterized by integer type
 */
template <class I>
struct Fraction {
    typedef I value_type;

    operator double() const noexcept;
    operator float() const noexcept;

    I num;
    I den;
};

template <class I>
inline Fraction<I>::operator double() const noexcept
{
    return static_cast<double>(num) / static_cast<double>(den);
}

template <class I>
inline Fraction<I>::operator float() const noexcept
{
    return static_cast<float>(num) / static_cast<float>(den);
}

/**
   @brief Characteristics of IEEE754 floating point representations
 */
template <class F>
struct FP_traits;

template <>
struct FP_traits<double> {
    typedef double type;
    typedef uint64_t same_size_int;
    static_assert(sizeof(type) == sizeof(same_size_int),
        "Unexpected size of floating point type");
    static constexpr int e_bits = 11;
    static constexpr int m_bits = 52;
    static constexpr int e_offset = -1023;
};

template <>
struct FP_traits<float> {
    typedef float type;
    typedef uint32_t same_size_int;
    static_assert(sizeof(type) == sizeof(same_size_int),
        "Unexpected size of floating point type");
    static constexpr int e_bits = 8;
    static constexpr int m_bits = 23;
    static constexpr int e_offset = -127;
};

/**
   @brief Get the sign part of a IEEE754 floating point number.

   The number is reconstructed as `(-1^sign)*(1+mantissa)*(2^exponent)`.
   See also `fp_exponent` and `fp_mantissa`.
 */
template <class F>
inline bool fp_sign(F x)
{
    typedef FP_traits<F> T;
    union {
        F real;
        typename T::same_size_int integer;
    } u;
    u.real = x;
    return ((u.integer >> (T::e_bits + T::m_bits)) & 1) != 0;
}

/**
   @brief Get the exponent part of a IEEE754 floating point number.

   The number is reconstructed as `(-1^sign)*(1+mantissa)*(2^exponent)`.
   See also `fp_sign` and `fp_mantissa`.

   It is a faster way of computing `floor(log2(abs(x)))`.
 */
template <class F>
inline int fp_exponent(F x)
{
    typedef FP_traits<F> T;
    union {
        F real;
        typename T::same_size_int integer;
    } u;
    u.real = x;
    int ex = (u.integer >> T::m_bits) & ((1u << T::e_bits) - 1);
    return ex + T::e_offset;
}

/**
   @brief Get the mantissa part of a IEEE754 floating point number.
   The number is reconstructed as `(-1^sign)*(1+mantissa)*(2^exponent)`.
   See also `fp_sign` and `fp_exponent`.
 */
template <class F>
inline Fraction<uint64_t> fp_mantissa(F x)
{
    typedef FP_traits<F> T;
    union {
        F real;
        typename T::same_size_int integer;
    } u;
    u.real = x;
    Fraction<uint64_t> f;
    f.den = uint64_t { 1 } << T::m_bits;
    f.num = u.integer & (f.den - 1);
    return f;
}

/**
   @brief Reconstruct a IEEE754 floating point number from its parts.

   The parts must be in their range of validity.
 */
template <class F>
inline F fp_from_parts(bool sgn, int ex, uint64_t mant)
{
    typedef FP_traits<F> T;
    typedef typename T::same_size_int I;
    union {
        F real;
        I integer;
    } u;
    u.integer = mant | (static_cast<I>(ex - T::e_offset) << T::m_bits) | (static_cast<I>(sgn) << (T::e_bits + T::m_bits));
    return u.real;
}

template <class F>
inline bool fp_naninf(F x)
{
    typedef FP_traits<F> T;
    typedef typename T::same_size_int I;
    union {
        F real;
        I integer;
    } u;
    u.real = x;
    const auto all_ones = ((1u << T::e_bits) - 1);
    const auto ex = (u.integer >> T::m_bits) & all_ones;
    return ex == all_ones;
}

template <class Type>
bool hasNanInf(absl::Span<Type> span)
{
    for (const auto& x : span)
        if (fp_naninf(x))
            return true;

    return false;
}

template <class Type>
bool isReasonableAudio(absl::Span<Type> span)
{
    for (const auto& x : span)
        if (x < -10.0f || x > 10.0f)
            return false;

    return true;
}

/**
 * @brief Compute the Kaiser window
 *
 * @param b Kaiser parameter beta
 * @param window Span of real which receives the window
 */
template <class T>
void kaiserWindow(double b, absl::Span<T> window)
{
    double i0b = i0(b);
    for (size_t i = 0, n = window.size(); i < n; ++i) {
        double x = i / static_cast<double>(n - 1);
        double t = x + x - 1.0;
        window[i] = static_cast<T>(i0(b * std::sqrt(1.0 - t * t)) / i0b);
    }
}

/**
 * @brief Compute a single point of the Kaiser window
 * This is less efficient than calculating the whole window at once.
 *
 * @param b Kaiser parameter beta
 * @param x Point to evaluate, normalized in 0 to 1
 */
inline double kaiserWindowSinglePoint(double b, double x)
{
    double t = x + x - 1.0;
    return i0(b * std::sqrt(1.0 - t * t)) / i0(b);
}

/**
 * @brief Compute the cardinal sine
 */
template <class T>
T sinc(T x)
{
    return (x == T(0)) ? T(1) : (std::sin(x) / x);
}

/**
 * @brief Compute the normalized cardinal sine
 */
template <class T>
T normalizedSinc(T x)
{
    return sinc(pi<T>() * x);
}

/**
 * @brief Finds the minimum size of 2 spans
 *
 * @tparam T
 * @tparam U
 * @param span1
 * @param span2
 * @return constexpr size_t
 */
template <class T, class U>
constexpr size_t minSpanSize(absl::Span<T>& span1, absl::Span<U>& span2)
{
    return min(span1.size(), span2.size());
}

/**
 * @brief Finds the minimum size of a list of spans.
 *
 * @tparam T
 * @tparam Others
 * @param first
 * @param others
 * @return constexpr size_t
 */
template <class T, class... Others>
constexpr size_t minSpanSize(absl::Span<T>& first, Others... others)
{
    return min(first.size(), minSpanSize(others...));
}

template <class T>
constexpr bool _checkSpanSizes(size_t size, absl::Span<T>& span1)
{
    return span1.size() == size;
}

template <class T, class... Others>
constexpr bool _checkSpanSizes(size_t size, absl::Span<T>& span1, Others... others)
{
    return span1.size() == size && _checkSpanSizes(size, others...);
}

/**
 * @brief Check that all spans of a compile time list have the same size
 *
 * @tparam T
 * @tparam Others
 * @param first
 * @param others
 * @return constexpr size_t
 */
template <class T, class... Others>
constexpr bool checkSpanSizes(const absl::Span<T>& span1, Others... others)
{
    return _checkSpanSizes(span1.size(), others...);
}

#define CHECK_SPAN_SIZES(...) SFIZZ_CHECK(checkSpanSizes(__VA_ARGS__))

class ScopedRoundingMode {
public:
    ScopedRoundingMode() = delete;
    ScopedRoundingMode(int newRoundingMode)
        : savedFloatMode(std::fegetround())
    {
        std::fesetround(newRoundingMode);
    }
    ~ScopedRoundingMode()
    {
        std::fesetround(savedFloatMode);
    }

private:
    const int savedFloatMode;
};

/**
 * @brief A low-quality random number generator guaranteed to be very fast
 */
class fast_rand {
public:
    typedef uint32_t result_type;

    fast_rand() noexcept
    {
    }

    explicit fast_rand(uint32_t value) noexcept
        : mem(value)
    {
    }

    static constexpr uint32_t min() noexcept
    {
        return 0;
    }

    static constexpr uint32_t max() noexcept
    {
        return std::numeric_limits<uint32_t>::max();
    }

    uint32_t operator()() noexcept
    {
        uint32_t next = mem * 1664525u + 1013904223u; // Numerical Recipes
        mem = next;
        return next;
    }

    void seed(uint32_t value = 0) noexcept
    {
        mem = value;
    }

    void discard(unsigned long long z) noexcept
    {
        for (unsigned long long i = 0; i < z; ++i)
            operator()();
    }

private:
    uint32_t mem = 0;
};

/**
 * @brief A uniform real distribution guaranteed to be very fast
 */
template <class T>
class fast_real_distribution {
public:
    static_assert(std::is_floating_point<T>::value, "The type must be floating point.");

    typedef T result_type;

    fast_real_distribution(T a, T b)
        : a_(a), b_(b), k_(b - a)
    {
    }

    template <class G>
    T operator()(G& g) const
    {
        return a_ + (g() - T(G::min())) * (k_ / T(G::max() - G::min()));
    }

    T a() const noexcept
    {
        return a_;
    }

    T b() const noexcept
    {
        return b_;
    }

    T min() const noexcept
    {
        return a_;
    }

    T max() const noexcept
    {
        return b_;
    }

private:
    T a_;
    T b_;
    T k_;
};

/**
 * @brief Global random singletons
 *
 * TODO: could be moved into a singleton class holder
 *
 */
namespace Random {
static fast_rand randomGenerator;
} // namespace Random

/**
 * @brief Generate normally distributed noise.
 *
 * This sums the output of N uniform random generators.
 * The higher the N, the better is the approximation of a normal distribution.
 */
template <class T, unsigned N = 4>
class fast_gaussian_generator {
    static_assert(N > 1, "Invalid quality setting");

public:
    explicit fast_gaussian_generator(float mean, float variance, uint32_t initialSeed = Random::randomGenerator())
    {
        mean_ = mean;
        gain_ = variance / std::sqrt(N / 3.0);
        seed(initialSeed);
    }

    void seed(uint32_t s)
    {
        seeds_[0] = s;
        for (unsigned i = 1; i < N; ++i) {
            s += s * 1664525u + 1013904223u;
            seeds_[i] = s;
        }
    }

    float operator()() noexcept
    {
        float sum = 0;
        for (unsigned i = 0; i < N; ++i) {
            uint32_t next = seeds_[i] * 1664525u + 1013904223u;
            seeds_[i] = next;
            sum += static_cast<int32_t>(next) * (1.0f / (1ll << 31));
        }
        return mean_ + gain_ * sum;
    }

private:
    std::array<uint32_t, N> seeds_ {{}};
    float mean_ { 0 };
    float gain_ { 0 };
};
