/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_matrix_misc_h
#define	cml_mathlib_matrix_misc_h

#include <cml/scalar/promotion.h>
#include <cml/vector/fwd.h>
#include <cml/matrix/fwd.h>

/** @defgroup mathlib_matrix_misc Miscellaneous Matrix Functions */

namespace cml {

/** @addtogroup mathlib_matrix_misc */
/*@{*/

/** Compute the trace of the upper-left 2x2 submatrix of @c m.
 *
 * @throws minimum_matrix_size_error if @c m is dynamically-sized, and is
 * not at least 2x2.  If @c m is fixed-size, the size is checked at
 * run-time.
 */
template<class Sub> auto trace_2x2(const readable_matrix<Sub>& m)
  -> value_type_trait_of_t<Sub>;

/** Compute the trace of the upper-left 3x3 submatrix of @c m.
 *
 * @throws minimum_matrix_size_error if @c m is dynamically-sized, and is
 * not at least 3x3.  If @c m is fixed-size, the size is checked at
 * run-time.
 */
template<class Sub> auto trace_3x3(const readable_matrix<Sub>& m)
  -> value_type_trait_of_t<Sub>;


/** Generate an upper-left 3x3 skew-symmetric submatrix @c m from the
 * elements of @c v, accounting for the basis orientation.
 *
 * @throws minimum_matrix_size_error if @c m is dynamically-sized, and is
 * not at least 3x3.  If @c m is fixed-size, the size is checked at
 * run-time.
 *
 * @throws vector_size_error if @c v is dynamically-sized, and is not 3D.
 * If @c v is fixed-size, the size is checked at run-time.
 */
template<class Sub1, class Sub2> void matrix_skew_symmetric(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& v);

/** Generate an upper-left 2x2 skew-symmetric submatrix in @c m from the
 * value of @c s, accounting for the basis orientation.
 *
 * @throws minimum_matrix_size_error if @c m is dynamically-sized, and is
 * not at least 2x2.  If @c m is fixed-size, the size is checked at
 * run-time.
 */
template<class Sub1, class Scalar> void matrix_skew_symmetric(
  writable_matrix<Sub1>& m, const Scalar& s);


/** Invert a 3D matrix composed of a rotation and translation.
 *
 * @throws minimum_matrix_size_error if @c m is dynamically-sized, and is
 * not at least 3x4.  If @c m is fixed-size, the size is checked at
 * run-time.
 */

/*@}*/

} // namespace cml

#define __CML_MATHLIB_MATRIX_MISC_TPP
#include <cml/mathlib/matrix/misc.tpp>
#undef __CML_MATHLIB_MATRIX_MISC_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
