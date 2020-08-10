/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_detail_lu_h
#define	cml_matrix_detail_lu_h

#include <cml/matrix/fwd.h>

namespace cml {
namespace detail {

/** In-place LU decomposition using Doolittle's method.
 *
 * @tparam Sub Derived output matrix type.
 *
 * @warning Without pivoting, this is numerically stable only for
 * diagonally dominant matrices.
 *
 * @note It is up to the caller to ensure @c M is a square matrix.
 */
template<class Sub> inline void
lu_inplace(writable_matrix<Sub>& M);

/** In-place LU decomposition using partial pivoting for non-singular
 * square matrices.  @c order contains the new row order after pivoting,
 * and the diagonal elements are those of the upper matrix.  This
 * implements the algorithm from Cormen, Leiserson, Rivest, '96.
 *
 * @tparam Sub Derived output matrix type.
 * @tparam order Row order array.
 *
 * @returns 1 if no pivots or an even number of pivots are performed, -1 if
 * an odd number of pivots are performed, 0 if M is singular.
 */
template<class Sub, class OrderArray> inline int
lu_pivot_inplace(writable_matrix<Sub>& M, OrderArray& order);

} // namespace detail
} // namespace cml

#define __CML_MATRIX_DETAIL_LU_TPP
#include <cml/matrix/detail/lu.tpp>
#undef __CML_MATRIX_DETAIL_LU_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
