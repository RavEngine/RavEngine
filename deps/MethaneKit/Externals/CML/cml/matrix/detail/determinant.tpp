/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_DETAIL_DETERMINANT_TPP
#error "matrix/detail/determinant.tpp not included correctly"
#endif

#include <vector>
#include <cml/matrix/temporary.h>
#include <cml/matrix/detail/lu.h>

namespace cml {
namespace detail {
namespace {

template<class Matrix> inline auto
diagonal_product(const Matrix& A) -> value_type_of_t<Matrix>
{
  auto D = A(0,0);
  for(int i = 1; i < A.rows(); ++ i) D *= A(i,i);
  return  D;
}

} // namespace

/** 2x2 determinant implementation. */
template<class Sub> inline auto
determinant(const readable_matrix<Sub>& M, int_c<2>)
-> value_type_trait_of_t<Sub>
{
  return M(0,0)*M(1,1) - M(1,0)*M(0,1);
}

/** 3x3 determinant implementation. */
template<class Sub> inline auto
determinant(const readable_matrix<Sub>& M, int_c<3>)
-> value_type_trait_of_t<Sub>
{
  return
    M(0,0) * (M(1,1)*M(2,2) - M(1,2)*M(2,1)) +
    M(0,1) * (M(1,2)*M(2,0) - M(1,0)*M(2,2)) +
    M(0,2) * (M(1,0)*M(2,1) - M(1,1)*M(2,0))
    ;
}

/** 4x4 determinant implementation. */
template<class Sub> inline auto
determinant(const readable_matrix<Sub>& M, int_c<4>)
-> value_type_trait_of_t<Sub>
{
  /* Common cofactors: */
  auto m_22_33_23_32 = M(2,2)*M(3,3) - M(2,3)*M(3,2);
  auto m_23_30_20_33 = M(2,3)*M(3,0) - M(2,0)*M(3,3);
  auto m_20_31_21_30 = M(2,0)*M(3,1) - M(2,1)*M(3,0);
  auto m_21_32_22_31 = M(2,1)*M(3,2) - M(2,2)*M(3,1);
  auto m_23_31_21_33 = M(2,3)*M(3,1) - M(2,1)*M(3,3);
  auto m_20_32_22_30 = M(2,0)*M(3,2) - M(2,2)*M(3,0);

  auto d00 = M(0,0) *(
    M(1,1) * m_22_33_23_32 +
    M(1,2) * m_23_31_21_33 +
    M(1,3) * m_21_32_22_31
    );

  auto d01 = M(0,1) *(
    M(1,0) * m_22_33_23_32 +
    M(1,2) * m_23_30_20_33 +
    M(1,3) * m_20_32_22_30
    );

  auto d02 = M(0,2) *(
    M(1,0) * -m_23_31_21_33 +
    M(1,1) * m_23_30_20_33 +
    M(1,3) * m_20_31_21_30
    );

  auto d03 = M(0,3)*(
    M(1,0) * m_21_32_22_31 +
    M(1,1) * - m_20_32_22_30 +
    M(1,2) * m_20_31_21_30
    );

  return d00 - d01 + d02 - d03;
}

/** Determinant implementation for statically-sized square matrices with
 * dimension greater than 4, using a pivoting algorithm to compute the
 * result.
 *
 * @note It is up to the caller to ensure @c M is a square matrix.
 */
template<class Sub, int N> inline auto
determinant(const readable_matrix<Sub>& M, int_c<N>)
-> value_type_trait_of_t<Sub>
{
  temporary_of_t<Sub> A(M);
  std::array<int,N> order;
  int sign = lu_pivot_inplace(A, order);

  /* Compute the determinant from the diagonals: */
  return sign * diagonal_product(A);
}

/** Determinant implementation for dynamically-sized matrices.  This
 * dispatches to a small matrix implementation when the dimension of @c M
 * is no more than 4.  Otherwise, the general pivoting implementation is
 * used.
 *
 * @note It is up to the caller to ensure @c M is a square matrix.
 */
template<class Sub> inline auto
determinant(const readable_matrix<Sub>& M, int_c<-1>)
-> value_type_trait_of_t<Sub>
{
  /* Size of matrix */
  int N = M.rows();

  /* Use the small matrix determinant if possible: */
  switch(N) {
    case 2: return determinant(M, int_c<2>()); break;
    case 3: return determinant(M, int_c<3>()); break;
    case 4: return determinant(M, int_c<4>()); break;
  }

  temporary_of_t<Sub> A(M);
  std::vector<int> order(A.rows());
  int sign = lu_pivot_inplace(A, order);

  /* Compute the determinant from the diagonals: */
  return sign * diagonal_product(A);
}

} // namespace detail
} // namespace cml


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
