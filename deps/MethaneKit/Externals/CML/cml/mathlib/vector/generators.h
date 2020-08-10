/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_vector_generators_h
#define	cml_mathlib_vector_generators_h

#include <cml/vector/fixed_compiled.h>

/** @defgroup mathlib_vector_generators Vector Generator Functions */

namespace cml {

/** @addtogroup mathlib_vector_generators */
/*@{*/

/** @defgroup mathlib_vector_generators_zero Zero Vector Generators */
/*@{*/

/** Return a fixed-size double-precision N-d zero vector. */
template<int N> inline auto zero()
-> vector<double, compiled<N>>
{
  return vector<double, compiled<N>>().zero();
}

/** Return the 2D zero vector */
inline auto zero_2D() -> decltype(zero<2>()) { return zero<2>(); }

/** Return the 3D zero vector */
inline auto zero_3D() -> decltype(zero<3>()) { return zero<3>(); }

/** Return the 4D zero vector */
inline auto zero_4D() -> decltype(zero<4>()) { return zero<4>(); }

/*@}*/

/** @defgroup mathlib_vector_generators_cardinal Cardinal Axis Generators */
/*@{*/

/** Return a fixed-size double-precision N-d cardinal axis by index. */
template<int N> inline auto axis(int i)
-> vector<double, compiled<N>>
{
  return vector<double, compiled<N>>().cardinal(i);
}

/** Return a 2D cardinal axis by index. */
inline auto axis_2D(int i) -> decltype(axis<2>(i)) {
  return axis<2>(i);
}

/** Return the 2D x-axis. */
inline auto x_axis_2D() -> decltype(axis<2>(0)) {
  return axis<2>(0);
}

/** Return the 2D y-axis. */
inline auto y_axis_2D() -> decltype(axis<2>(1)) {
  return axis<2>(1);
}

/** Return a 3D cardinal axis by index. */
inline auto axis_3D(int i) -> decltype(axis<3>(i)) {
  return axis<3>(i);
}

/** Return the 3D x-axis. */
inline auto x_axis_3D() -> decltype(axis<3>(0)) {
  return axis<3>(0);
}

/** Return the 3D y-axis. */
inline auto y_axis_3D() -> decltype(axis<3>(1)) {
  return axis<3>(1);
}

/** Return the 3D z-axis. */
inline auto z_axis_3D() -> decltype(axis<3>(2)) {
  return axis<3>(2);
}

/*@}*/

/*@}*/

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
