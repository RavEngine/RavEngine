/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_matrix_size_checking_h
#define	cml_mathlib_matrix_size_checking_h

#include <cml/common/mpl/int_c.h>
#include <cml/common/mpl/enable_if_array.h>
#include <cml/common/exception.h>
#include <cml/matrix/fwd.h>

namespace cml {

/** Exception thrown when run-time size checking is enabled, and a matrix
 * is not sized to hold an affine transformation.
 */
struct affine_matrix_size_error : std::runtime_error {
  affine_matrix_size_error()
    : std::runtime_error("incorrect affine matrix size") {}
};

/** Front-end for both compile-time and run-time 2D affine matrix size
 * checking.  A row-basis matrix must be at least 3x2, while a column-basis
 * matrix must be at least 2x3.
 *
 * @tparam Sub the actual type of the matrix expression.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 2D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub> void check_affine_2D(const readable_matrix<Sub>& m);

/** Front-end for both compile-time and run-time 3D affine matrix size
 * checking.  A row-basis matrix must be at least 4x3, while a column-basis
 * matrix must be at least 3x4.
 *
 * @tparam Sub the actual type of the matrix expression.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 3D affine transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub> void check_affine_3D(const readable_matrix<Sub>& m);

/** Front-end for both compile-time and run-time affine matrix size
 * checking.  A row-basis matrix must have size (N,N) or (N+1,N), while
 * a column-basis matrix must have size (N,N) or (N,N+1).
 *
 * @tparam Sub the actual type of the matrix expression.
 *
 * @throws affine_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for an affine transformation.  If @c
 * m is fixed-size, the size is checked at compile-time.
 */
template<class Sub> void check_affine(const readable_matrix<Sub>& m);

/** Front-end for both compile-time and run-time 2D linear matrix size
 * checking.  A linear matrix must be at least 2x2.
 *
 * @tparam Sub the actual type of the matrix expression.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 2D linear transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub> void check_linear_2D(const readable_matrix<Sub>& m);

/** Front-end for both compile-time and run-time 3D linear matrix size
 * checking.  A linear matrix must be at least 3x3.
 *
 * @tparam Sub the actual type of the matrix expression.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not sized for a 3D linear transformation.  If
 * @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub> void check_linear_3D(const readable_matrix<Sub>& m);

} // namespace cml

#define __CML_MATHLIB_MATRIX_SIZE_CHECKING_TPP
#include <cml/mathlib/matrix/size_checking.tpp>
#undef __CML_MATHLIB_MATRIX_SIZE_CHECKING_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
