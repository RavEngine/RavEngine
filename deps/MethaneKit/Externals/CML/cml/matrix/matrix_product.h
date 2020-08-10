/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_matrix_product_h
#define	cml_matrix_matrix_product_h

#include <cml/matrix/readable_matrix.h>
#include <cml/matrix/promotion.h>

namespace cml {

template<class LeftMatrix, class RightMatrix>
using matrix_product_t = matrix_inner_product_promote_t<actual_operand_type_of_t<LeftMatrix>, actual_operand_type_of_t<RightMatrix>>;

/** Multiply two matrices, and return the result as a temporary. */
template<class LeftMatrix, class RightMatrix,
         enable_if_matrix_t<LeftMatrix>* = nullptr,
         enable_if_matrix_t<RightMatrix>* = nullptr>
auto operator*(LeftMatrix&& sub1, RightMatrix&& sub2)
-> matrix_product_t<decltype(sub1), decltype(sub2)>;

} // namespace cml

#define __CML_MATRIX_MATRIX_PRODUCT_TPP
#include <cml/matrix/matrix_product.tpp>
#undef __CML_MATRIX_MATRIX_PRODUCT_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
