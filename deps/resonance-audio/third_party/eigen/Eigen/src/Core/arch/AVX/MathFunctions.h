// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2014 Pedro Gonnet (pedro.gonnet@gmail.com)
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef EIGEN_MATH_FUNCTIONS_AVX_H
#define EIGEN_MATH_FUNCTIONS_AVX_H

/* The sin and cos functions of this file are loosely derived from
 * Julien Pommier's sse math library: http://gruntthepeon.free.fr/ssemath/
 */

#include "../../InternalHeaderCheck.h"

namespace Eigen {

namespace internal {

template <>
EIGEN_DEFINE_FUNCTION_ALLOWING_MULTIPLE_DEFINITIONS Packet8f
psin<Packet8f>(const Packet8f& _x) {
  return psin_float(_x);
}

template <>
EIGEN_DEFINE_FUNCTION_ALLOWING_MULTIPLE_DEFINITIONS Packet8f
pcos<Packet8f>(const Packet8f& _x) {
  return pcos_float(_x);
}

template <>
EIGEN_DEFINE_FUNCTION_ALLOWING_MULTIPLE_DEFINITIONS Packet8f
plog<Packet8f>(const Packet8f& _x) {
  return plog_float(_x);
}

template <>
EIGEN_DEFINE_FUNCTION_ALLOWING_MULTIPLE_DEFINITIONS Packet4d
plog<Packet4d>(const Packet4d& _x) {
  return plog_double(_x);
}

template <>
EIGEN_DEFINE_FUNCTION_ALLOWING_MULTIPLE_DEFINITIONS Packet8f
plog2<Packet8f>(const Packet8f& _x) {
  return plog2_float(_x);
}

template <>
EIGEN_DEFINE_FUNCTION_ALLOWING_MULTIPLE_DEFINITIONS Packet4d
plog2<Packet4d>(const Packet4d& _x) {
  return plog2_double(_x);
}

template<> EIGEN_DEFINE_FUNCTION_ALLOWING_MULTIPLE_DEFINITIONS
Packet8f plog1p<Packet8f>(const Packet8f& _x) {
  return generic_plog1p(_x);
}

template<> EIGEN_DEFINE_FUNCTION_ALLOWING_MULTIPLE_DEFINITIONS
Packet8f pexpm1<Packet8f>(const Packet8f& _x) {
  return generic_expm1(_x);
}

// Exponential function. Works by writing "x = m*log(2) + r" where
// "m = floor(x/log(2)+1/2)" and "r" is the remainder. The result is then
// "exp(x) = 2^m*exp(r)" where exp(r) is in the range [-1,1).
template <>
EIGEN_DEFINE_FUNCTION_ALLOWING_MULTIPLE_DEFINITIONS Packet8f
pexp<Packet8f>(const Packet8f& _x) {
  return pexp_float(_x);
}

// Hyperbolic Tangent function.
template <>
EIGEN_DEFINE_FUNCTION_ALLOWING_MULTIPLE_DEFINITIONS Packet8f
ptanh<Packet8f>(const Packet8f& _x) {
  return internal::generic_fast_tanh_float(_x);
}

// Exponential function for doubles.
template <>
EIGEN_DEFINE_FUNCTION_ALLOWING_MULTIPLE_DEFINITIONS Packet4d
pexp<Packet4d>(const Packet4d& _x) {
  return pexp_double(_x);
}


// Notice that for newer processors, it is counterproductive to use Newton
// iteration for square root. In particular, Skylake and Zen2 processors
// have approximately doubled throughput of the _mm_sqrt_ps instruction
// compared to their predecessors.
template <> EIGEN_DEFINE_FUNCTION_ALLOWING_MULTIPLE_DEFINITIONS
Packet8f psqrt<Packet8f>(const Packet8f& _x) {
  return _mm256_sqrt_ps(_x);
}
template <> EIGEN_DEFINE_FUNCTION_ALLOWING_MULTIPLE_DEFINITIONS
Packet4d psqrt<Packet4d>(const Packet4d& _x) {
  return _mm256_sqrt_pd(_x);
}


// Even on Skylake, using Newton iteration is a win for reciprocal square root.
#if EIGEN_FAST_MATH
template<> EIGEN_DEFINE_FUNCTION_ALLOWING_MULTIPLE_DEFINITIONS
Packet8f prsqrt<Packet8f>(const Packet8f& a) {
  // _mm256_rsqrt_ps returns -inf for negative denormals.
  // _mm512_rsqrt**_ps returns -NaN for negative denormals.  We may want
  // consistency here.
  // const Packet8f rsqrt = pselect(pcmp_lt(a, pzero(a)),
  //                                pset1<Packet8f>(-NumTraits<float>::quiet_NaN()),
  //                                _mm256_rsqrt_ps(a));
  return generic_rsqrt_newton_step<Packet8f, /*Steps=*/1>::run(a, _mm256_rsqrt_ps(a));
}

template<> EIGEN_STRONG_INLINE Packet8f preciprocal<Packet8f>(const Packet8f& a) {
  return generic_reciprocal_newton_step<Packet8f, /*Steps=*/1>::run(a, _mm256_rcp_ps(a));
}

#endif


F16_PACKET_FUNCTION(Packet8f, Packet8h, psin)
F16_PACKET_FUNCTION(Packet8f, Packet8h, pcos)
F16_PACKET_FUNCTION(Packet8f, Packet8h, plog)
F16_PACKET_FUNCTION(Packet8f, Packet8h, plog2)
F16_PACKET_FUNCTION(Packet8f, Packet8h, plog1p)
F16_PACKET_FUNCTION(Packet8f, Packet8h, pexpm1)
F16_PACKET_FUNCTION(Packet8f, Packet8h, pexp)
F16_PACKET_FUNCTION(Packet8f, Packet8h, ptanh)
F16_PACKET_FUNCTION(Packet8f, Packet8h, psqrt)
F16_PACKET_FUNCTION(Packet8f, Packet8h, prsqrt)
F16_PACKET_FUNCTION(Packet8f, Packet8h, preciprocal)

template <>
EIGEN_STRONG_INLINE Packet8h pfrexp(const Packet8h& a, Packet8h& exponent) {
  Packet8f fexponent;
  const Packet8h out = float2half(pfrexp<Packet8f>(half2float(a), fexponent));
  exponent = float2half(fexponent);
  return out;
}

template <>
EIGEN_STRONG_INLINE Packet8h pldexp(const Packet8h& a, const Packet8h& exponent) {
  return float2half(pldexp<Packet8f>(half2float(a), half2float(exponent)));
}

BF16_PACKET_FUNCTION(Packet8f, Packet8bf, psin)
BF16_PACKET_FUNCTION(Packet8f, Packet8bf, pcos)
BF16_PACKET_FUNCTION(Packet8f, Packet8bf, plog)
BF16_PACKET_FUNCTION(Packet8f, Packet8bf, plog2)
BF16_PACKET_FUNCTION(Packet8f, Packet8bf, plog1p)
BF16_PACKET_FUNCTION(Packet8f, Packet8bf, pexpm1)
BF16_PACKET_FUNCTION(Packet8f, Packet8bf, pexp)
BF16_PACKET_FUNCTION(Packet8f, Packet8bf, ptanh)
BF16_PACKET_FUNCTION(Packet8f, Packet8bf, psqrt)
BF16_PACKET_FUNCTION(Packet8f, Packet8bf, prsqrt)
BF16_PACKET_FUNCTION(Packet8f, Packet8bf, preciprocal)

template <>
EIGEN_STRONG_INLINE Packet8bf pfrexp(const Packet8bf& a, Packet8bf& exponent) {
  Packet8f fexponent;
  const Packet8bf out = F32ToBf16(pfrexp<Packet8f>(Bf16ToF32(a), fexponent));
  exponent = F32ToBf16(fexponent);
  return out;
}

template <>
EIGEN_STRONG_INLINE Packet8bf pldexp(const Packet8bf& a, const Packet8bf& exponent) {
  return F32ToBf16(pldexp<Packet8f>(Bf16ToF32(a), Bf16ToF32(exponent)));
}

}  // end namespace internal

}  // end namespace Eigen

#endif  // EIGEN_MATH_FUNCTIONS_AVX_H
