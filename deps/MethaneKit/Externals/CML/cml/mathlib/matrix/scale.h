/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_matrix_scale_h
#define	cml_mathlib_matrix_scale_h

#include <cml/vector/fwd.h>
#include <cml/matrix/fwd.h>

/** @defgroup mathlib_matrix_scale Matrix Scale Functions */

namespace cml {

/** @addtogroup mathlib_matrix_scale */
/*@{*/

/** @defgroup mathlib_matrix_scale_2D 2D Matrix Scale Functions */
/*@{*/

/** Initialize a non-uniform 2D scaling matrix with scales @c e0 and @c e1.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 2D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub, class E0, class E1> void matrix_scale_2D(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1);

/** Initialize a non-uniform 2D scaling matrix, taking scales from the
 * elements of @c v.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 2D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 *
 * @throws vector_size_error at run-time if @c v is dynamically-sized, and
 * is not 2D.  If @c v is fixed-size, the size is checked at compile-time.
 */
template<class Sub1, class Sub2> void matrix_scale_2D(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& v);

/** Initialize a uniform 2D scaling matrix with scale @c e0.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 2D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub, class E0> void matrix_uniform_scale_2D(
  writable_matrix<Sub>& m, const E0& e0);

/** Initialize a non-uniform 2D scaling matrix with the reciprocals of
 * scales @c e0 and @c e1.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 2D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub, class E0, class E1> void matrix_inverse_scale_2D(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1);

/** Initialize a non-uniform 2D scaling matrix, taking scales from reciprocals
 * of the elements of @c v.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 2D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 *
 * @throws vector_size_error at run-time if @c v is dynamically-sized, and
 * is not 2D.  If @c v is fixed-size, the size is checked at compile-time.
 */
template<class Sub1, class Sub2> void matrix_inverse_scale_2D(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& v);

/*@}*/


/** @defgroup mathlib_matrix_scale_3D 3D Matrix Scale Functions */
/*@{*/

/** Initialize a non-uniform 3D scaling matrix with scales @c e0, @c e1,
 * and @c e2.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 3D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub, class E0, class E1, class E2> void matrix_scale(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1, const E2& e2);

/** Initialize a non-uniform 3D scaling matrix, taking scales from the
 * elements of @c v.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 3D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 *
 * @throws vector_size_error at run-time if @c v is dynamically-sized, and
 * is not 3D.  If @c v is fixed-size, the size is checked at compile-time.
 */
template<class Sub1, class Sub2> void matrix_scale(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& v);

/** Initialize a uniform 3D scaling matrix with scale @c e0.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 3D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub, class E0> void matrix_uniform_scale(
  writable_matrix<Sub>& m, const E0& e0);

/** Initialize a non-uniform 3D scaling matrix with the reciprocals of
 * scales @c e0, @c e1, and @c e2.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 3D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub, class E0, class E1, class E2> void matrix_inverse_scale(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1, const E2& e2);

/** Initialize a non-uniform 3D scaling matrix, taking scales from reciprocals
 * of the elements of @c v.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 3D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 *
 * @throws vector_size_error at run-time if @c v is dynamically-sized, and
 * is not 3D.  If @c v is fixed-size, the size is checked at compile-time.
 */
template<class Sub1, class Sub2> void matrix_inverse_scale(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& v);

/*@}*/

/*@}*/

} // namespace cml

#define __CML_MATHLIB_MATRIX_SCALE_TPP
#include <cml/mathlib/matrix/scale.tpp>
#undef __CML_MATHLIB_MATRIX_SCALE_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
