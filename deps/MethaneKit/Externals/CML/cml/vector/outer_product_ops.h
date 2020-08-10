/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_outer_product_ops_h
#define	cml_vector_outer_product_ops_h

#include <cml/vector/outer_product_node.h>

namespace cml {

/** Compute the outer product of vectors @c sub1 and @c sub2 as a matrix
 * expression node.
 */
template<class Sub1, class Sub2,
  enable_if_vector_t<Sub1>* = nullptr, enable_if_vector_t<Sub2>* = nullptr>
inline auto outer(Sub1&& sub1, Sub2&& sub2)
-> outer_product_node<
actual_operand_type_of_t<decltype(sub1)>,
actual_operand_type_of_t<decltype(sub2)>>
{
  static_assert(std::is_same<
    decltype(sub1), decltype(std::forward<Sub1>(sub1))>::value,
    "internal error: unexpected expression type (sub1)");
  static_assert(std::is_same<
    decltype(sub2), decltype(std::forward<Sub2>(sub2))>::value,
    "internal error: unexpected expression type (sub2)");

  /* Deduce the operand types of the subexpressions (&, const&, &&): */
  typedef actual_operand_type_of_t<decltype(sub1)> sub1_type;
  typedef actual_operand_type_of_t<decltype(sub2)> sub2_type;
  return outer_product_node<
    sub1_type, sub2_type>((sub1_type) sub1, (sub2_type) sub2);
}

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
