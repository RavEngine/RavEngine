/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_detail_determinant_h
#define	cml_matrix_detail_determinant_h

#include <cml/common/mpl/int_c.h>
#include <cml/common/traits.h>
#include <cml/matrix/fwd.h>

namespace cml {
namespace detail {

/** 2x2 determinant implementation. */
template<class Sub> inline auto
determinant(const readable_matrix<Sub>& M, int_c<2>)
-> value_type_trait_of_t<Sub>;

/** 3x3 determinant implementation. */
template<class Sub> inline auto
determinant(const readable_matrix<Sub>& M, int_c<3>)
-> value_type_trait_of_t<Sub>;

/** 4x4 determinant implementation. */
template<class Sub> inline auto
determinant(const readable_matrix<Sub>& M, int_c<4>)
-> value_type_trait_of_t<Sub>;

/** Determinant implementation for statically-sized square matrices with
 * dimension greater than 4, using a pivoting algorithm to compute the
 * result.
 *
 * @note It is up to the caller to ensure @c M is a square matrix.
 */
template<class Sub, int N> inline auto
determinant(const readable_matrix<Sub>& M, int_c<N>)
-> value_type_trait_of_t<Sub>;

/** Determinant implementation for dynamically-sized matrices.  This
 * dispatches to a small matrix implementation when the dimension of @c M
 * is no more than 4.  Otherwise, the general pivoting implementation is
 * used.
 *
 * @note It is up to the caller to ensure @c M is a square matrix.
 */
template<class Sub> inline auto
determinant(const readable_matrix<Sub>& M, int_c<-1>)
-> value_type_trait_of_t<Sub>;

} // namespace detail
} // namespace cml

#define __CML_MATRIX_DETAIL_DETERMINANT_TPP
#include <cml/matrix/detail/determinant.tpp>
#undef __CML_MATRIX_DETAIL_DETERMINANT_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
