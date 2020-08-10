/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_lu_h
#define	cml_matrix_lu_h

#include <array>
#include <vector>
#include <cml/common/type_util.h>
#include <cml/common/array_size_of.h>
#include <cml/vector/temporary.h>
#include <cml/matrix/temporary.h>

namespace cml {

/** Sepcializable class to hold results from LU decomposition with partial
 * pivoting.
 */
template<class Matrix, class Enable = void> struct lu_pivot_result;

/** Results from partial-pivoting LU decomposition of a fixed-size matrix. */
template<class Matrix>
struct lu_pivot_result<Matrix, enable_if_fixed_size_t<matrix_traits<Matrix>>>
{
  static const int N = array_rows_of_c<matrix_traits<Matrix>>::value;

  Matrix			lu;
  std::array<int,N>		order;
  int				sign;

  explicit lu_pivot_result(const Matrix& M) : lu(M), order() {}
};

/** Results from partial-pivoting LU decomposition of a dynamic-size matrix. */
template<class Matrix>
struct lu_pivot_result<Matrix, enable_if_dynamic_size_t<matrix_traits<Matrix>>>
{
  Matrix			lu;
  std::vector<int>		order;
  int				sign;

  explicit lu_pivot_result(const Matrix& M) : lu(M), order(M.rows()) {}
};


/** Compute the LU decomposition of M, with partial pivoting.  The result
 * is returned in an lu_pivot_result.
 *
 * @note if @c result.sign is 0, the input matrix is singular.
 */
template<class Sub> auto
lu_pivot(const readable_matrix<Sub>& M)
-> lu_pivot_result< temporary_of_t<Sub> >;

/** In-place computation of the partial-pivoting LU decomposition of @c
 * result.lu.
 *
 * @note if @c result.sign is 0, the input matrix is singular.
 */
template<class Matrix> void
lu_pivot(lu_pivot_result<Matrix>& result);

/** Compute the LU decomposition of @c M using Doolittle's method,
 * returning the result as a temporary matrix.
 *
 * @warning Without pivoting, this is numerically stable only for
 * diagonally dominant matrices.
 *
 * @note This is for compatibility with CML1.
 */
template<class Sub> auto
lu(const readable_matrix<Sub>& M) -> temporary_of_t<Sub>;


/** Solve @c LUx = @c y for @c x, and return @c x as a temporary
 * vector.  @c LU must be a square LU decomposition, and @c y must have the
 * same number of elements as @c LU has rows.
 *
 * @note This is for compatibility with CML1.
 */
template<class LUSub, class BSub> auto
lu_solve(const readable_matrix<LUSub>& LU, const readable_vector<BSub>& y)
-> temporary_of_t<BSub>;

/** Solve @c LUx = @c b for @c x.  @c LU must be a square LU
 * decomposition, and @c y and @c x must have the same number of elements
 * as @c LU has rows.
 */
template<class LUSub, class XSub, class BSub> void
lu_solve(const readable_matrix<LUSub>& LU,
  writable_vector<XSub>& x, const readable_vector<BSub>& b);

/** Solve @c LUx = @c Pb for @c x, where the partial-pivot LU
 * decomposition is provided as lu_pivot_result, and @c x is returned as a
 * vector temporary.  @c b must have the same number of elements as @c
 * lup.lu has rows.
 *
 * @throws std::invalid_argument @c lup.sign is 0.
 */
template<class Matrix, class BSub> auto
lu_solve(const lu_pivot_result<Matrix>& lup, const readable_vector<BSub>& b)
-> temporary_of_t<BSub>;

/** Solve @c LUx = @c Pb for @c x, where the partial-pivot LU
 * decomposition is provided as lu_pivot_result.  @c b and @c x must have
 * the same number of elements as @c lup.lu has rows.
 *
 * @note @c x can be the same vector as @c b.
 *
 * @throws std::invalid_argument @c lup.sign is 0.
 */
template<class Matrix, class XSub, class BSub> void
lu_solve(const lu_pivot_result<Matrix>& lup,
  writable_vector<XSub>& x, const readable_vector<BSub>& b);

} // namespace cml

#define __CML_MATRIX_LU_TPP
#include <cml/matrix/lu.tpp>
#undef __CML_MATRIX_LU_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
