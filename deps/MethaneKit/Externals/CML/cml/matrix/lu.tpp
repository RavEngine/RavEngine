/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_LU_TPP
#error "matrix/lu.tpp not included correctly"
#endif

#include <cml/vector/writable_vector.h>
#include <cml/matrix/size_checking.h>
#include <cml/matrix/detail/lu.h>

namespace cml {

template<class Sub> inline auto
lu_pivot(const readable_matrix<Sub>& M)
-> lu_pivot_result< temporary_of_t<Sub> >
{
  cml::check_square(M);
  lu_pivot_result<temporary_of_t<Sub>> result(M);
  result.sign = detail::lu_pivot_inplace(result.lu, result.order);
  return result;
}

template<class Matrix> inline void
lu_pivot(lu_pivot_result<Matrix>& result)
{
  cml::check_square(result.lu);
  result.sign = detail::lu_pivot_inplace(result.lu, result.order);
}

template<class Sub> inline auto
lu(const readable_matrix<Sub>& M) -> temporary_of_t<Sub>
{
  cml::check_square(M);
  temporary_of_t<Sub> LU(M);
  detail::lu_inplace(LU);
  return LU;
}

template<class LUSub, class BSub> inline auto
lu_solve(const readable_matrix<LUSub>& LU, const readable_vector<BSub>& b)
-> temporary_of_t<BSub>
{
  temporary_of_t<BSub> x; detail::check_or_resize(x, b);
  lu_solve(LU, x, b);
  return x;
}

template<class LUSub, class XSub, class BSub> inline void
lu_solve(const readable_matrix<LUSub>& LU,
  writable_vector<XSub>& x, const readable_vector<BSub>& b)
{
  typedef value_type_trait_of_t<BSub>			value_type;

  cml::check_square(LU);
  cml::check_same_inner_size(LU, x);
  cml::check_same_inner_size(LU, b);

  int N = b.size();

  /* Solve Ly = b for y by forward substitution.  The entries below the
   * diagonal of LU correspond to L, understood to be below a diagonal of
   * 1's:
   */
  temporary_of_t<XSub> y; detail::check_or_resize(y, b);
  for(int i = 0; i < N; ++ i) {
    value_type sum(0);
    for(int j = 0; j < i; ++ j) sum += LU(i,j)*y[j];
    y[i] = b[i] - sum;
  }

  /* Solve Ux = y for x by backward substitution.  The entries at and above
   * the diagonal of LU correspond to U:
   */
  for(int i = N-1; i >= 0; --i) {
    value_type sum(0);
    for(int j = i+1; j < N; ++j) sum += LU(i,j)*x[j];
    x[i] = (y[i] - sum)/LU(i,i);
  }

  /* Done. */
}


template<class Matrix, class BSub> inline auto
lu_solve(const lu_pivot_result<Matrix>& lup, const readable_vector<BSub>& b)
-> temporary_of_t<BSub>
{
  temporary_of_t<BSub> x; detail::check_or_resize(x, b);
  lu_solve(lup, x, b);
  return x;
}

template<class Matrix, class XSub, class BSub> inline void
lu_solve(const lu_pivot_result<Matrix>& lup,
  writable_vector<XSub>& x, const readable_vector<BSub>& b)
{
  typedef value_type_trait_of_t<BSub>			value_type;

  cml::check_same_inner_size(lup.lu, x);
  cml::check_same_inner_size(lup.lu, b);
  cml_require(lup.sign != 0,
    std::invalid_argument, "lup.sign == 0 (singular matrix?)");

  int N = b.size();
  const auto& LU = lup.lu;
  const auto& P = lup.order;

  /* Solve Ly = Pb for c by forward substitution.  The entries below the
   * diagonal of LU correspond to L, understood to be below a diagonal of
   * 1's:
   */
  temporary_of_t<XSub> y; detail::check_or_resize(y, b);
  for(int i = 0; i < N; ++ i) {
    value_type sum(0);
    for(int j = 0; j < i; ++ j) sum += LU(i,j)*y[j];
    y[i] = b[P[i]] - sum;
  }

  /* Solve Ux = c for x by backward substitution.  The entries at and above
   * the diagonal of LU correspond to U:
   */
  for(int i = N-1; i >= 0; --i) {
    value_type sum(0);
    for(int j = i+1; j < N; ++j) sum += LU(i,j)*x[j];
    x[i] = (y[i] - sum)/LU(i,i);
  }

  /* Done. */
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
