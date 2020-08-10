/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_vector_product_h
#define	cml_matrix_vector_product_h

#include <cml/matrix/promotion.h>

namespace cml {

/** Multiply a vector by a matrix, and return the vector result as a
 * temporary.
 */
template<class Sub1, class Sub2,
  enable_if_matrix_t<Sub1>* = nullptr,
  enable_if_vector_t<Sub2>* = nullptr>
auto operator*(Sub1&& sub1, Sub2&& sub2)
-> matrix_inner_product_promote_t<
  actual_operand_type_of_t<decltype(sub1)>,
  actual_operand_type_of_t<decltype(sub2)>>;

/** Multiply a matrix by a vector, and return the vector result as a
 * temporary.
 */
template<class Sub1, class Sub2,
  enable_if_vector_t<Sub1>* = nullptr,
  enable_if_matrix_t<Sub2>* = nullptr>
auto operator*(Sub1&& sub1, Sub2&& sub2)
-> matrix_inner_product_promote_t<
  actual_operand_type_of_t<decltype(sub1)>,
  actual_operand_type_of_t<decltype(sub2)>>;

} // namespace cml

#define __CML_MATRIX_VECTOR_PRODUCT_TPP
#include <cml/matrix/vector_product.tpp>
#undef __CML_MATRIX_VECTOR_PRPDUCT_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
