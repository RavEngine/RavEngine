/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_VECTOR_PRODUCT_TPP
#error "matrix/vector_product.tpp not included correctly"
#endif

#include <cml/vector/detail/resize.h>
#include <cml/matrix/size_checking.h>

namespace cml {

template<class Sub1, class Sub2,
  enable_if_matrix_t<Sub1>*, enable_if_vector_t<Sub2>*>
inline auto operator*(Sub1&& sub1, Sub2&& sub2)
-> matrix_inner_product_promote_t<
  actual_operand_type_of_t<decltype(sub1)>,
  actual_operand_type_of_t<decltype(sub2)>>
{
  typedef matrix_inner_product_promote_t<
    actual_operand_type_of_t<decltype(sub1)>,
    actual_operand_type_of_t<decltype(sub2)>>		result_type;

  cml::check_same_inner_size(sub1, sub2);

  result_type v;
  detail::resize(v, array_rows_of(sub1));
  for(int i = 0; i < sub1.rows(); ++ i) {
    auto m = sub1(i,0) * sub2[0];
    for(int k = 1; k < sub2.size(); ++ k) m += sub1(i,k) * sub2[k];
    v[i] = m;
  }
  return v;
}

template<class Sub1, class Sub2,
  enable_if_vector_t<Sub1>*, enable_if_matrix_t<Sub2>*>
inline auto operator*(Sub1&& sub1, Sub2&& sub2)
-> matrix_inner_product_promote_t<
  actual_operand_type_of_t<decltype(sub1)>,
  actual_operand_type_of_t<decltype(sub2)>>
{
  typedef matrix_inner_product_promote_t<
    actual_operand_type_of_t<decltype(sub1)>,
    actual_operand_type_of_t<decltype(sub2)>>		result_type;

  cml::check_same_inner_size(sub1, sub2);

  result_type v;
  detail::resize(v, array_cols_of(sub2));
  for(int j = 0; j < sub2.cols(); ++ j) {
    auto m = sub1[0] * sub2(0,j);
    for(int k = 1; k < sub1.size(); ++ k) m += sub1[k] * sub2(k,j);
    v[j] = m;
  }
  return v;
}

// TODO Both of these can (and should) be refactored to return a vector
// expression node.  This requires automatic temporary generation within
// the expression tree first, though.

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
