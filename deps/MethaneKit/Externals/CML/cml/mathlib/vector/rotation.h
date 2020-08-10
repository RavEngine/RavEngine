/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_vector_rotation_h
#define	cml_mathlib_vector_rotation_h

#include <cml/vector/promotion.h>

/** @defgroup mathlib_vector_rotation Vector Rotation Functions */

namespace cml {

/** @addtogroup mathlib_vector_rotation_3D */
/*@{*/

/** Rotate by @c angle a 3D vector @c v about a unit-length vector @c n.
 *
 * @throws vector_size_error at run-time if @c v or @c n is
 * dynamically-sized, and are not 3D.  If fixed-size, the sizes are checked
 * at compile-time.
 */
template<class Sub1, class Sub2, class E> auto rotate_vector(
  const readable_vector<Sub1>& v, const readable_vector<Sub2>& n,
  const E& angle) -> vector_promote_t<Sub1,Sub2>;

/*@}*/

} // namespace cml

#define __CML_MATHLIB_VECTOR_ROTATION_TPP
#include <cml/mathlib/vector/rotation.tpp>
#undef __CML_MATHLIB_VECTOR_ROTATION_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
