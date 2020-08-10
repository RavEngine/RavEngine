/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_matrix_invert_h
#define	cml_mathlib_matrix_invert_h

#include <cml/matrix/fwd.h>

namespace cml {

/** Invert a 2D affine transformation consisting of a rotation and a
 * translation only.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 2D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub> void matrix_invert_RT_only_2D(writable_matrix<Sub>& m);

/** Invert a 3D affine transformation consisting of a rotation and a
 * translation only.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 3D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub> void matrix_invert_RT_only(writable_matrix<Sub>& m);

/** Invert an n-D affine transformation consisting of a rotation and a
 * translation only.
 *
 * @throws affine_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized an affine transformation.  If @c m
 * is fixed-size, the size is checked at compile-time.
 */
template<class Sub> void matrix_invert_RT(writable_matrix<Sub>& m);

} // namespace cml

#define __CML_MATHLIB_MATRIX_INVERT_TPP
#include <cml/mathlib/matrix/invert.tpp>
#undef __CML_MATHLIB_MATRIX_INVERT_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
