/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_matrix_transform_h
#define	cml_mathlib_matrix_transform_h

#include <cml/vector/fwd.h>
#include <cml/matrix/fwd.h>
#include <cml/mathlib/constants.h>

/** @defgroup mathlib_matrix_transform Matrix Transform Functions */

namespace cml {

/** @addtogroup mathlib_matrix_transform */
/*@{*/

/** @defgroup mathlib_matrix_lookat_3D 3D Matrix "Look-At" Functions */
/*@{*/

/** Build a matrix representing a 'look at' view transform given the eye
 * position, target, reference up vector, and the handedness.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class SubEye, class SubTarget, class SubUp> void
matrix_look_at(writable_matrix<Sub>& m,
    const readable_vector<SubEye>& position,
    const readable_vector<SubTarget>& target,
    const readable_vector<SubUp>& up,
    AxisOrientation handedness);

/** Build a matrix representing a left-handed 'look at' view transform
 * given the eye position, target, and reference up vector.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class SubEye, class SubTarget, class SubUp> void
matrix_look_at_LH(writable_matrix<Sub>& m,
    const readable_vector<SubEye>& position,
    const readable_vector<SubTarget>& target,
    const readable_vector<SubUp>& up);

/** Build a matrix representing a right-handed 'look at' view transform
 * given the eye position, target, and reference up vector.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class SubEye, class SubTarget, class SubUp> void
matrix_look_at_RH(writable_matrix<Sub>& m,
    const readable_vector<SubEye>& position,
    const readable_vector<SubTarget>& target,
    const readable_vector<SubUp>& up);

/*@}*/

/** @defgroup mathlib_matrix_linear_3D 3D Matrix Linear Transform Functions */
/*@{*/

/** Build a matrix @c m from the 3x3 linear transform part of matrix @c l.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 3x3.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub1, class Sub2> inline void matrix_linear_transform(
  writable_matrix<Sub1>& m, const readable_matrix<Sub2>& l);

/*@}*/

/** @defgroup mathlib_matrix_affine_3D 3D Matrix Affine Transform Functions */
/*@{*/

/** Build an affine transform @c m from an axis-angle pair and a
 * translation.  If @c normalize is true, @c axis will be made a unit
 * vector before computing rotations.  The default is false.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized as a 3D affine matrix.  If @c m is
 * fixed-size, the size is checked at compile-time.
 *
 * @throws vector_size_error at run-time if @c axis is dynamically-sized,
 * and is not 3D.  If @c axis is fixed-size, the size is checked at
 * compile-time.
 *
 * @throws vector_size_error at run-time if @c translation is
 * dynamically-sized, and is not 3D.  If @c translation is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class ASub, class E, class TSub> inline void
matrix_affine_transform(writable_matrix<Sub>& m,
    const readable_vector<ASub>& axis, const E& angle,
    const readable_vector<TSub>& translation,
    bool normalize = false);

/** Build an affine transform @c m from a linear matrix and a translation.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized as a 3D affine matrix.  If @c m is
 * fixed-size, the size is checked at compile-time.
 *
 * @throws minimum_matrix_size_error at run-time if @c linear is
 * dynamically-sized, and is not 3x3.  If @c linear is fixed-size, the size
 * is checked at compile-time.
 *
 * @throws vector_size_error at run-time if @c translation is
 * dynamically-sized, and is not 3D.  If @c translation is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class LSub, class TSub> inline void
matrix_affine_transform(
    writable_matrix<Sub>& m, const readable_matrix<LSub>& linear,
    const readable_vector<TSub>& translation);

/*@}*/

/*@}*/

/*@}*/

} // namespace cml

#define __CML_MATHLIB_MATRIX_TRANSFORM_TPP
#include <cml/mathlib/matrix/transform.tpp>
#undef __CML_MATHLIB_MATRIX_TRANSFORM_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
