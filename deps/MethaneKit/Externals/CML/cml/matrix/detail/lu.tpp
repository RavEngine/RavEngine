/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_DETAIL_LU_TPP
#error "matrix/detail/lu.tpp not included correctly"
#endif

#include <cml/common/traits.h>

namespace cml {
namespace detail {

template<class Sub> inline void
lu_inplace(writable_matrix<Sub>& M)
{
  typedef value_type_trait_of_t<Sub>			value_type;

  int N = M.rows();
  for(int k = 0; k < N; ++ k) {

    /* Compute the upper triangle: */
    for(int j = k; j < N; ++ j) {
      value_type sum(0);
      for(int p = 0; p < k; ++ p) sum += M(k,p)*M(p,j);
      M(k,j) -= sum;
    }

    /* Compute the lower triangle: */
    for(int i = k+1; i < N; ++ i) {
      value_type sum(0);
      for(int p = 0; p < k; ++ p) sum += M(i,p)*M(p,k);
      M(i,k) = (M(i,k) - sum) / M(k,k);
    }
  }
}

template<class Sub, class OrderArray> inline int
lu_pivot_inplace(writable_matrix<Sub>& M, OrderArray& order)
{
  typedef value_type_trait_of_t<Sub>			value_type;
  typedef traits_of_t<value_type>			value_traits;

  /* Initialize the order: */
  int N = M.rows();
  for(int i = 0; i < N; ++ i) order[i] = i;

  /* For each column: */
  int flag = 1;
  for(int k = 0; k < N-1; ++ k) {

    /* Find the next pivot row: */
    int row = k;
    value_type max = M(k,k);
    for(int i = k+1; i < N; ++ i) {
      value_type mag = value_traits::fabs(M(i,k));
      if(mag > max) {
	max = mag;
	row = i;
      }
    }

    /* Check for a singular matrix: */
    if(max < value_traits::epsilon()) return 0;
    // XXX should be configurable?

    /* Update order and swap rows: */
    if(row != k) {
      std::swap(order[k], order[row]);
      for(int i = 0; i < N; ++ i) std::swap(M(k,i), M(row,i));
      flag = - flag;
    }

    /* Compute the Schur complement: */
    for(int i = k+1; i < N; ++ i) {
      M(i,k) /= M(k,k);
      for(int j = k+1; j < N; ++ j)
       	M(i,j) -= M(i,k)*M(k,j);
    }
  }

  /* Done: */
  return flag;
}

} // namespace detail
} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
