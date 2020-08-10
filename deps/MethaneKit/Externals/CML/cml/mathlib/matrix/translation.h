/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_matrix_translation_h
#define	cml_mathlib_matrix_translation_h

#include <cml/vector/fwd.h>
#include <cml/matrix/fwd.h>
#include <cml/mathlib/matrix/temporary.h>

/** @defgroup mathlib_matrix_translation Matrix Translation Functions */

namespace cml {

/** @addtogroup mathlib_matrix_translation */
/*@{*/

/** @defgroup mathlib_matrix_translation_2D 2D Matrix Translation Functions */
/*@{*/

/** Set the translation of a 2D affine transformation, @c m, to @c e0 and
 * @c e1.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 2D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub, class E0, class E1> void matrix_set_translation_2D(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1);

/** Set the translation of a 2D affine transformation, @c m, to the 2D
 * vector @c v.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 2D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 *
 * @throws vector_size_error at run-time if @c v is dynamically-sized, and
 * is not 2D.  If @c v is fixed-size, the size is checked at compile-time.
 */
template<class Sub1, class Sub2> void matrix_set_translation_2D(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& v);


/** Get the translation vector of a 2D affine transformation as two scalar
 * values.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 2D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub, class E0, class E1> void matrix_get_translation_2D(
  const readable_matrix<Sub>& m, E0& e0, E1& e1);

/** Get the translation of a 2D affine transformation as a 2D vector.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 2D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub> auto matrix_get_translation_2D(
  const readable_matrix<Sub>& m) -> n_basis_vector_of_t<Sub,2>;


/** Initialize a 2D translation matrix, @c m, from @c e0 and @c e1.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 2D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub, class E0, class E1> void matrix_translation_2D(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1);

/** Initialize a 2D translation matrix, @c m, from the 2D vector @c v.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 2D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 *
 * @throws vector_size_error at run-time if @c v is dynamically-sized, and
 * is not 2D.  If @c v is fixed-size, the size is checked at compile-time.
 */
template<class Sub1, class Sub2> void matrix_translation_2D(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& v);

/*@}*/


/** @defgroup mathlib_matrix_translation_3D 3D Matrix Translation Functions */
/*@{*/

/** Set the translation of a 3D affine transformation, @c m, to @c e0, @c
 * e1, and @c e2.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 3D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub, class E0, class E1, class E2> void matrix_set_translation(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1, const E2& e2);

/** Set the translation of a 3D affine transformation, @c m, to @c e0, @c
 * e1, and 0.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 3D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub, class E0, class E1> void matrix_set_translation(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1);

/** Set the translation of a 3D affine transformation, @c m, to the
 * 2D or 3D vector @c v (if @c v is 2D the last element is 0).
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 3D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 *
 * @throws vector_size_error at run-time if @c v is dynamically-sized, and
 * is not 2D or 3D.  If @c v is fixed-size, the size is checked at
 * compile-time.
 */
template<class Sub1, class Sub2> void matrix_set_translation(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& v);


/** Get the translation vector of a 3D affine transformation as three
 * scalar values.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 3D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub, class E0, class E1, class E2> void matrix_get_translation(
  const readable_matrix<Sub>& m, E0& e0, E1& e1, E2& e2);

/** Get the translation of a 3D affine transformation as a 3D vector.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 3D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub> auto matrix_get_translation(
  const readable_matrix<Sub>& m) -> n_basis_vector_of_t<Sub,3>;


/** Initialize a 3D translation matrix, @c m, from @c e0, @c e1, and @c e2.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 3D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub, class E0, class E1, class E2> void matrix_translation(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1, const E2& e2);

/** Initialize a 3D translation matrix, @c m, from @c e0, @c e1, and 0.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 3D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub, class E0, class E1> void matrix_translation(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1);

/** Initialize a 3D translation matrix, @c m, from the 3D vector @c v.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 3D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 *
 * @throws vector_size_error at run-time if @c v is dynamically-sized, and
 * is not 3D.  If @c v is fixed-size, the size is checked at compile-time.
 */
template<class Sub1, class Sub2> void matrix_translation(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& v);

/*@}*/

/*@}*/

} // namespace cml

#define __CML_MATHLIB_MATRIX_TRANSLATION_TPP
#include <cml/mathlib/matrix/translation.tpp>
#undef __CML_MATHLIB_MATRIX_TRANSLATION_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
