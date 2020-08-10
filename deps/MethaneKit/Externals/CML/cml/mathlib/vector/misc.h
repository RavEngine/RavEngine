/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_vector_misc_h
#define	cml_mathlib_vector_misc_h

#include <cml/scalar/promotion.h>
#include <cml/vector/promotion.h>
#include <cml/vector/temporary.h>

/** @defgroup mathlib_vector_misc Miscellaneous Vector Functions */

namespace cml {

/** @addtogroup mathlib_vector_misc */
/*@{*/

/** Project @c u onto another vector @c v.
 *
 * @throws minimum_vector_size_error at run-time if @c u or @c v has
 * fewer than one element. If both vectors are fixed-size, the size is
 * checked at compile-time.
 *
 * @throws incompatible_vector_size_error at run-time if @c u and @v n 
 * are different sizes, and at least one is dynamically-sized.  If both
 * vectors are fixed-size, the size is checked at compile-time.
 */
template<class Sub1, class Sub2> auto project_to_vector(
  const readable_vector<Sub1>& u, const readable_vector<Sub2>& v)
-> vector_promote_t<Sub1, Sub2>;

/** Project @c v onto a hyperplane with normal @c n.
 *
 * @note @c n is assumed to be normalized.
 *
 * @throws minimum_vector_size_error at run-time if @c v or @c n has
 * fewer than one element. If both vectors are fixed-size, the size is
 * checked at compile-time.
 *
 * @throws incompatible_vector_size_error at run-time if @c v and @c n 
 * are different sizes, and at least one is dynamically-sized.  If both
 * vectors are fixed-size, the size is checked at compile-time.
 */
template<class Sub1, class Sub2> auto project_to_hplane(
  const readable_vector<Sub1>& v, const readable_vector<Sub2>& n)
-> vector_promote_t<Sub1, Sub2>;

/** Return a vector counter-clockwise perpendicular to @c v.
 *
 * @throws vector_size_error at run-time if @c v is dynamically-sized and
 * is not a 2D vector.  The size is checked at compile-time if @c v is
 * fixed-size.
 */
template<class Sub> auto perp(const readable_vector<Sub>& v)
-> temporary_of_t<Sub>;

/** Compute the Manhattan (city-block) distance between @c v1 and @c v2.
 *
 * @throws minimum_vector_size_error at run-time if @c v1 or @c v2 has
 * fewer than one element. If both vectors are fixed-size, the size is
 * checked at compile-time.
 *
 * @throws incompatible_vector_size_error at run-time if @c v1 and @c v2
 * are different sizes, and at least one is dynamically-sized.  If both
 * vectors are fixed-size, the size is checked at compile-time.
 */
template<class Sub1, class Sub2> auto manhattan_distance(
  const readable_vector<Sub1>& v1, const readable_vector<Sub2>& v2)
-> value_type_trait_promote_t<Sub1, Sub2>;

/*@}*/

} // namespace cml

#define __CML_MATHLIB_VECTOR_MISC_TPP
#include <cml/mathlib/vector/misc.tpp>
#undef __CML_MATHLIB_VECTOR_MISC_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
