/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_determinant_h
#define	cml_matrix_determinant_h

#include <cml/common/traits.h>
#include <cml/matrix/fwd.h>

namespace cml {

/** Compute the determinant of square matrix @c M.  For matrices of
 * dimension less than 4, the determinant is computed directly.  For larger
 * matrices, the determinant is computed from the LU decomposition with
 * partial pivoting.
 *
 * @throws non_square_matrix_error at run-time if the matrix is
 * dynamically-sized and not square.  Fixed-size matrices are checked at
 * compile-time.
 */
template<class Sub> auto
determinant(const readable_matrix<Sub>& M) -> value_type_trait_of_t<Sub>;

} // namespace cml

#define __CML_MATRIX_DETERMINANT_TPP
#include <cml/matrix/determinant.tpp>
#undef __CML_MATRIX_DETERMINANT_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
