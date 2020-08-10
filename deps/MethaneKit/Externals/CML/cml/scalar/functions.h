/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_scalar_functions_h
#define	cml_scalar_functions_h

#include <algorithm>
#include <random>
#include <cml/scalar/constants.h>
#include <cml/scalar/traits.h>

namespace cml {

/** Sign (-1, 0, 1) of @c value as type @c T. */
template<typename T> inline T sign(T value) {
  return value < T(0) ? (-T(1)) : (value > T(0) ? T(1) : T(0));
}

/** Clamp input value to the range [min, max]. */
template<typename T> inline T clamp(T value, T min, T max) {
  return std::max((std::min) (value, max), min);
}

/** Wrap input value to the range [min,max]. */
template<typename T> inline T wrap(T value, T min, T max) {
  max -= min;
  value = scalar_traits<T>::fmod(value - min, max);
  if (value < T(0)) { value += max; }
  return min + value;
}

/** Test input value for inclusion in [min, max]. */
template<typename T> inline bool in_range(T value, T min, T max) {
  return !(value < min) && !(value > max);
}

/** Map input value from [min1, max1] to [min2, max2]. */
template<typename T> inline T map_range(
  T value, T min1, T max1, T min2, T max2
  )
{
  return min2 + ((value - min1) / (max1 - min1)) * (max2 - min2);
}

/** Wrap scalar_traits<T>::acos() and clamp argument to [-1, 1]. */
template<typename T> inline T acos_safe(T theta) {
  return T(scalar_traits<T>::acos(clamp(theta, T(-1), T(1))));
}

/** Wrap scalar_traits<T>::asin() and clamp argument to [-1, 1]. */
template<typename T> inline T asin_safe(T theta) {
  return T(scalar_traits<T>::asin(clamp(theta, T(-1), T(1))));
}

/** Wrap scalar_traits<T>::sqrt() and clamp argument to [0, inf). */
template<typename T> inline T sqrt_safe(T value) {
  return T(scalar_traits<T>::sqrt(std::max(value, T(0))));
}


/** Square a value. */
template<typename T> inline T sqr(T value) { return value * value; }

/** Cube a value. */
template<typename T> inline T cub(T value) { return value * value * value; }

/** Inverse square root. */
template<typename T> inline T inv_sqrt(T value) {
  return T(1) / scalar_traits<T>::sqrt(value);
}


/** Convert radians to degrees. */
template<typename T> inline T deg(T theta) {
  return theta * constants<T>::deg_per_rad();
}

/** Convert degrees to radians. */
template<typename T> inline T rad(T theta) {
  return theta * constants<T>::rad_per_deg();
}


/** Uniformly random integer in the range [min, max]. */
template<typename T> inline T random_integer(T min, T max) {
  std::default_random_engine gen(std::rand());
  std::uniform_int_distribution<T> dis(min,max);
  return dis(gen);
}

/** Uniformly random binary (0,1) value. */
inline int random_binary() { return random_integer<int>(0,1); }

/** Uniformly random polar (-1,1) value. */
inline int random_polar() { return random_binary() ? 1 : -1; }

/* Uniformly distributed random real number in the range [min, max] */
template<typename T> inline T random_real(T min, T max) {
  std::default_random_engine gen(std::rand());
  std::uniform_real_distribution<T> dis(min,max);
  return dis(gen);
}

/** Uniformly distributed random real in [0,1]. */
inline double random_unit() { return random_real<double>(0., 1.); }


/** Squared length in R2. */
template<typename T> inline T length_squared(T x, T y) {
  return x * x + y * y;
}

/** Squared length in R3. */
template<typename T> inline T length_squared(T x, T y, T z) {
  return x * x + y * y + z * z;
}

/** Length in R2. */
template<typename T> inline T length(T x, T y) {
  return scalar_traits<T>::sqrt(length_squared(x,y));
}

/** Length in R3. */
template<typename T> inline T length(T x, T y, T z) {
  return scalar_traits<T>::sqrt(length_squared(x,y,z));
}

/** @defgroup cml_scalar_indexing Indexing Functions
 *
 * The next few functions deal with indexing. next() and prev() are useful
 * for operations involving the vertices of a polygon or other cyclic set,
 * and cyclic_permutation() is used by various functions that deal with
 * axes or basis vectors in a generic way. As these functions are only
 * relevant for unsigned integer types, I've just used int, but there
 * may be reasons I haven't thought of that they should be templated.
 */
/*@{*/

/** Return next, with cycling, in a series of N non-negative integers. */
inline int next(int i, int N) {
  return (i + 1) % N;
}

/** Return previous, with cycling, in a series of N non-negative integers. */
inline int prev(int i, int N) {
  return i ? (i - 1) : (N - 1);
}

/** Cyclic permutation of the set { 0, 1 }, starting with 'first'. */
inline void cyclic_permutation(int first, int& i, int& j) {
  i = first;
  j = next(i, 2);
}

/** Cyclic permutation of the set { 0, 1, 2 }, starting with 'first'. */
inline void cyclic_permutation(int first, int& i, int& j, int& k)
{
  i = first;
  j = next(i, 3);
  k = next(j, 3);
}

/** Cyclic permutation of the set { 0, 1, 2, 3 }, starting with 'first'. */
inline void cyclic_permutation(
  int first, int& i, int& j, int& k, int& l)
{
  i = first;
  j = next(i, 4);
  k = next(j, 4);
  l = next(k, 4);
}

/*@}*/


/** Index of maximum of 2 values. */
template<typename T> inline int index_of_max(T a, T b) {
  return a > b ? 0 : 1;
}

/** Index of maximum of 2 values by magnitude. */
template<typename T> inline int index_of_max_abs(T a, T b) {
  auto fabs = &scalar_traits<T>::fabs;
  return index_of_max(fabs(a), fabs(b));
}

/** Index of minimum of 2 values. */
template<typename T> inline int index_of_min(T a, T b) {
  return a < b ? 0 : 1;
}

/** Index of minimum of 2 values by magnitude. */
template<typename T> inline int index_of_min_abs(T a, T b) {
  auto fabs = &scalar_traits<T>::fabs;
  return index_of_min(fabs(a), fabs(b));
}

/** Index of maximum of 3 values. */
template<typename T> inline int index_of_max(T a, T b, T c) {
  return a > b ? (c > a ? 2 : 0) : (b > c ? 1 : 2);
}

/** Index of maximum of 3 values by magnitude. */
template<typename T> inline int index_of_max_abs(T a, T b, T c) {
  auto fabs = &scalar_traits<T>::fabs;
  return index_of_max(fabs(a), fabs(b), fabs(c));
}

/** Index of minimum of 3 values. */
template<typename T> inline int index_of_min(T a, T b, T c) {
  return a < b ? (c < a ? 2 : 0) : (b < c ? 1 : 2);
}

/** Index of minimum of 3 values by magnitude. */
template<typename T> inline int index_of_min_abs(T a, T b, T c) {
  auto fabs = &scalar_traits<T>::fabs;
  return index_of_min(fabs(a), fabs(b), fabs(c));
}

/** Index of maximum of 4 values. */
template<typename T> inline int index_of_max(T a, T b, T c, T d) {
  if(a > b) {
    return (c > d) ? ((a > c) ? 0 : 2) : ((a > d) ? 0 : 3);
  } else {
    return (c > d) ? ((b > c) ? 1 : 2) : ((b > d) ? 1 : 3);
  }
}

/** Index of maximum of 4 values by magnitude. */
template<typename T> inline int index_of_max_abs(T a, T b, T c, T d) {
  auto fabs = &scalar_traits<T>::fabs;
  return index_of_max(fabs(a), fabs(b), fabs(c), fabs(d));
}

/** Index of minimum of 3 values. */
template<typename T> inline int index_of_min(T a, T b, T c, T d) {
  if(a < b) {
    return (c < d) ? ((a < c) ? 0 : 2) : ((a < d) ? 0 : 3);
  } else {
    return (c < d) ? ((b < c) ? 1 : 2) : ((b < d) ? 1 : 3);
  }
}

/** Index of minimum of 4 values by magnitude. */
template<typename T> inline int index_of_min_abs(T a, T b, T c, T d) {
  auto fabs = &scalar_traits<T>::fabs;
  return index_of_min(fabs(a), fabs(b), fabs(c), fabs(d));
}

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
