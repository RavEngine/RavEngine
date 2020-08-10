/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_matrix_projection_h
#define	cml_mathlib_matrix_projection_h

#include <cml/matrix/fwd.h>
#include <cml/mathlib/constants.h>

/** @defgroup mathlib_matrix_projection Projection Matrix Functions */

namespace cml {

/** @addtogroup mathlib_matrix_projection */
/*@{*/

/** @defgroup mathlib_matrix_orthographic Orthographic Projection */
/*@{*/

/** Build a matrix representing an orthographic projection given the
 * handedness, z-clipping range, and frustum bounds.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_orthographic(writable_matrix<Sub>& m,
  E left, E right, E bottom, E top, E n, E f,
  AxisOrientation handedness, ZClip z_clip);

/** Build a matrix representing a left-handed orthographic projection given
 * the z-clipping range and frustum bounds.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_orthographic_LH(writable_matrix<Sub>& m,
  E left, E right, E bottom, E top, E n, E f, ZClip z_clip);

/** Build a matrix representing a right-handed orthographic projection given
 * the z-clipping range and frustum bounds.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_orthographic_RH(writable_matrix<Sub>& m,
  E left, E right, E bottom, E top, E n, E f, ZClip z_clip);


/** Build a matrix representing a orthographic projection given the
 * handededness, z-clipping range, and frustum size.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_orthographic(writable_matrix<Sub>& m,
  E width, E height, E n, E f, AxisOrientation handedness, ZClip z_clip);

/** Build a matrix representing a left-handed orthographic projection given
 * the z-clipping range and frustum size.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_orthographic_LH(writable_matrix<Sub>& m,
  E width, E height, E n, E f, ZClip z_clip);

/** Build a matrix representing a right-handed orthographic projection given
 * the z-clipping range and frustum size.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_orthographic_RH(writable_matrix<Sub>& m,
  E width, E height, E n, E f, ZClip z_clip);

/*@}*/


/** @defgroup mathlib_matrix_perspective Perspective Projection */
/*@{*/

/** Build a matrix representing a perspective projection given the
 * handedness, z-clipping range, and frustum bounds.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_perspective(writable_matrix<Sub>& m,
  E left, E right, E bottom, E top, E n, E f,
  AxisOrientation handedness, ZClip z_clip);

/** Build a matrix representing a left-handed perspective projection given
 * the z-clipping range and frustum bounds.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_perspective_LH(writable_matrix<Sub>& m,
  E left, E right, E bottom, E top, E n, E f, ZClip z_clip);

/** Build a matrix representing a right-handed perspective projection given
 * the z-clipping range and frustum bounds.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_perspective_RH(writable_matrix<Sub>& m,
  E left, E right, E bottom, E top, E n, E f, ZClip z_clip);


/** Build a matrix representing a perspective projection given the
 * handededness, z-clipping range, and frustum size.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_perspective(writable_matrix<Sub>& m,
  E width, E height, E n, E f, AxisOrientation handedness, ZClip z_clip);

/** Build a matrix representing a left-handed perspective projection given
 * the z-clipping range and frustum size.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_perspective_LH(writable_matrix<Sub>& m,
  E width, E height, E n, E f, ZClip z_clip);

/** Build a matrix representing a right-handed perspective projection given
 * the z-clipping range and frustum size.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_perspective_RH(writable_matrix<Sub>& m,
  E width, E height, E n, E f, ZClip z_clip);


/** Build a matrix representing a perspective projection given the
 * handedness, z-clipping range, aspect ratio, horizontal field of view,
 * and near and far clip plane positions.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_perspective_xfov(writable_matrix<Sub>& m,
  E xfov, E aspect, E n, E f, AxisOrientation handedness, ZClip z_clip);

/** Build a matrix representing a left-hand perspective projection given
 * the z-clipping range, aspect ratio, horizontal field of view, and near
 * and far clip plane positions.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_perspective_xfov_LH(writable_matrix<Sub>& m,
  E xfov, E aspect, E n, E f, ZClip z_clip);

/** Build a matrix representing a right-hand perspective projection given
 * the z-clipping range, aspect ratio, horizontal field of view, and near
 * and far clip plane positions.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_perspective_xfov_RH(writable_matrix<Sub>& m,
  E xfov, E aspect, E n, E f, ZClip z_clip);


/** Build a matrix representing a perspective projection given the
 * handedness, z-clipping range, aspect ratio, vertical field of view,
 * and near and far clip plane positions.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_perspective_yfov(writable_matrix<Sub>& m,
  E yfov, E aspect, E n, E f, AxisOrientation handedness, ZClip z_clip);

/** Build a matrix representing a left-hand perspective projection given
 * the z-clipping range, aspect ratio, vertical field of view, and near
 * and far clip plane positions.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_perspective_yfov_LH(writable_matrix<Sub>& m,
  E yfov, E aspect, E n, E f, ZClip z_clip);

/** Build a matrix representing a right-hand perspective projection given
 * the z-clipping range, aspect ratio, vertical field of view, and near
 * and far clip plane positions.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 4x4.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_perspective_yfov_RH(writable_matrix<Sub>& m,
  E yfov, E aspect, E n, E f, ZClip z_clip);

/*@}*/

/*@}*/

} // namespace cml

#define __CML_MATHLIB_MATRIX_PROJECTION_TPP
#include <cml/mathlib/matrix/projection.tpp>
#undef __CML_MATHLIB_MATRIX_PROJECTION_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
