/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_vector_angle_h
#define	cml_mathlib_vector_angle_h

#include <cml/scalar/promotion.h>
#include <cml/vector/fwd.h>

/** @defgroup mathlib_vector_angle Vector Angle Functions */

namespace cml {

/** @addtogroup mathlib_vector_angle */
/*@{*/

/** Signed angle between two 2D vectors @c v1 and @c v2. */
template<class Sub1, class Sub2, class Sub3> inline auto signed_angle_2D(
  const readable_vector<Sub1>& v1, const readable_vector<Sub2>& v2)
-> value_type_trait_promote_t<Sub1,Sub2>;

/** Unsigned angle between two 2D vectors @c v1 and @c v2. */
template<class Sub1, class Sub2> inline auto unsigned_angle_2D(
  const readable_vector<Sub1>& v1, const readable_vector<Sub2>& v2)
-> value_type_trait_promote_t<Sub1,Sub2>;

/** Signed angle between two 3D vectors @c v1 and @c v2, relative @c
 * reference.
 */
template<class Sub1, class Sub2, class Sub3> inline auto signed_angle(
  const readable_vector<Sub1>& v1, const readable_vector<Sub2>& v2,
  const readable_vector<Sub3>& reference)
-> value_type_trait_promote_t<Sub1,Sub2,Sub3>;

/** Unsigned angle between two 3D vectors @c v1 and @c v2. */
template<class Sub1, class Sub2> inline auto unsigned_angle(
  const readable_vector<Sub1>& v1, const readable_vector<Sub2>& v2)
-> value_type_trait_promote_t<Sub1,Sub2>;

/*@}*/

} // namespace cml

#define __CML_MATHLIB_VECTOR_ANGLE_TPP
#include <cml/mathlib/vector/angle.tpp>
#undef __CML_MATHLIB_VECTOR_ANGLE_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
