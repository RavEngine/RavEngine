/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_MATRIX_PRODUCT_TPP
#error "matrix/matrix_product.tpp not included correctly"
#endif

#include <cml/matrix/detail/resize.h>
#include <cml/matrix/types.h>

#include <xmmintrin.h>

namespace cml {

template<class Test, class Reference>
using is_same_t = typename std::enable_if<std::is_same<cml::unqualified_type_t<Test>, Reference>::value>::type;

template<class Test, class Reference>
using is_different_t = typename std::enable_if<!std::is_same<cml::unqualified_type_t<Test>, Reference>::value>::type;

template<class LeftMatrix, class RightMatrix>
using matrix_product_t = matrix_inner_product_promote_t<actual_operand_type_of_t<LeftMatrix>, actual_operand_type_of_t<RightMatrix>>;

// General purpose matrix product implementation for all matrix types, except special matrix types with optimized implementation

template<class LeftMatrix, class RightMatrix,
         enable_if_matrix_t<LeftMatrix>* = nullptr,
         enable_if_matrix_t<RightMatrix>* = nullptr,
         is_different_t<LeftMatrix, matrix44f_r>* = nullptr,
         is_different_t<RightMatrix, matrix44f_r>* = nullptr>
inline auto matrix_product(LeftMatrix&& left, RightMatrix&& right)
-> matrix_product_t<decltype(left), decltype(right)>
{
  cml::check_same_inner_size(left, right);

  matrix_product_t<decltype(left), decltype(right)> M;
  detail::resize(M, array_rows_of(left), array_cols_of(right));
  for(int i = 0; i < M.rows(); ++ i) {
    for(int j = 0; j < M.cols(); ++ j) {
      auto m = left(i,0) * right(0,j);
      for(int k = 1; k < left.cols(); ++ k) m += left(i,k) * right(k,j);
      M(i,j) = m;
    }
  }
  return M;
}

// SSE optimized matrix product for float fixed matrices with row major alignment and row basis

template<class LeftMatrix, class RightMatrix,
         is_same_t<LeftMatrix, matrix44f_r>* = nullptr,
         is_same_t<RightMatrix, matrix44f_r>* = nullptr>
inline matrix44f_r matrix_product(LeftMatrix&& left, RightMatrix&& right)
{
  matrix44f_r  result;
  float const* p_left_row   = left.data();
  float*       p_result_row = result.data();

  __m128 right_cols[4];
  for (int col = 0; col < right.cols(); ++col) {
    right_cols[col] = _mm_loadu_ps(right.data() + col * right.rows());
  }

  for (int row = 0; row < left.rows(); ++row, p_left_row += left.cols(), p_result_row += result.cols()) {
    __m128 res_row = _mm_setzero_ps();
    for (int col = 0; col < left.cols(); ++col) {
      __m128 left_element = _mm_set1_ps(p_left_row[col]);
      res_row  = _mm_add_ps(res_row, _mm_mul_ps(left_element, right_cols[col]));
    }
    _mm_storeu_ps(p_result_row, res_row);
  }

  return result;
}

// Final matrix product template implementation

template<class LeftMatrix, class RightMatrix,
         enable_if_matrix_t<LeftMatrix>*,
         enable_if_matrix_t<RightMatrix>*>
inline auto operator*(LeftMatrix&& left, RightMatrix&& right)
-> matrix_product_t<decltype(left), decltype(right)>
{
  return matrix_product(left, right);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
