/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_frustum_h
#define	cml_mathlib_frustum_h

#include <cml/matrix/fwd.h>
#include <cml/mathlib/constants.h>

/** @defgroup mathlib_frustum Frustum Functions */

namespace cml {

/** @addtogroup mathlib_frustum */
/*@{*/

/** Extract the planes of a frustum given modelview and projection
 * matrices, and the near z-clipping range. The planes are normalized by
 * default, but this can be turned off with the 'normalize' argument.
 *
 * The planes are in ax+by+cz+d = 0 form, and are in the order:
 *     left
 *     right
 *     bottom
 *     top
 *     near
 *     far
 *
 * @throws minimum_matrix_size_error at run-time if @c modelview or @c
 * projection is dynamically-sized, and is not at least 4x4.  The size is
 * checked at compile-time for fixed-size matrices.
 *
 * @throws non_square_matrix_error at run-time if @c modelview or @c projection
 * is dynamically-sized and non-square.  The size is checked at
 * compile-time for fixed-size matrices.
 */
template<class Sub1, class Sub2, class E> void
extract_frustum_planes(
  const readable_matrix<Sub1>& modelview,
  const readable_matrix<Sub2>& projection,
  E planes[6][4], ZClip z_clip, bool normalize = true);

/** Extract the planes of a frustum from a matrix assumed to contain any
 * model and view transforms, followed by a projection transform with the
 * given near z-cliping range. The planes are normalized by default, but
 * this can be turned off with the 'normalize' argument.
 *
 * The planes are in ax+by+cz+d = 0 form, and are in the order:
 *     left
 *     right
 *     bottom
 *     top
 *     near
 *     far
 *
 * @throws minimum_matrix_size_error at run-time if @c m is dynamically-sized,
 * and is not at least 4x4.  The size is checked at compile-time for
 * fixed-size matrices.
 */
template<class Sub, class E> void
extract_frustum_planes(
  const readable_matrix<Sub>& m,
  E planes[6][4], ZClip z_clip, bool normalize = true);

/** Extract the near plane of a frustum given a concatenated modelview and
 * projection matrix @c m and the near z-clipping range. The plane is not
 * normalized.
 *
 * @note @c plane is in ax+by+cz+d = 0 form, and must have exactly 4
 * elements.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is dynamically-sized,
 * and is not at least 4x4.  The size is checked at compile-time for
 * fixed-size matrices.
 */
template<class Sub, class Plane> void
extract_near_frustum_plane(
  const readable_matrix<Sub>& m, Plane& plane, ZClip z_clip);

/*@}*/

} // namespace cml

#define __CML_MATHLIB_FRUSTUM_TPP
#include <cml/mathlib/frustum.tpp>
#undef __CML_MATHLIB_FRUSTUM_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
