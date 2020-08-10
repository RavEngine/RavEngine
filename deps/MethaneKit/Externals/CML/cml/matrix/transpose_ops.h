/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_transpose_ops_h
#define	cml_matrix_transpose_ops_h

#include <cml/matrix/transpose_node.h>

namespace cml {

/** Helper function to generate a matrix_unary_node from a matrix type
 * (i.e. derived from readable_matrix<>).
 */
template<class Sub, enable_if_matrix_t<Sub>* = nullptr>
inline auto make_matrix_transpose_node(Sub&& sub)
-> matrix_transpose_node<actual_operand_type_of_t<decltype(sub)>>
{
  static_assert(std::is_same<
    decltype(sub), decltype(std::forward<Sub>(sub))>::value,
    "internal error: unexpected expression type");

  /* Deduce the operand type of the subexpression (&, const&, &&): */
  typedef actual_operand_type_of_t<decltype(sub)> sub_type;
  return matrix_transpose_node<sub_type>((sub_type) sub);
}

template<class Sub, enable_if_matrix_t<Sub>* = nullptr>
inline auto transpose(Sub&& sub)
-> decltype(make_matrix_transpose_node(std::forward<Sub>(sub)))
{
  return make_matrix_transpose_node(std::forward<Sub>(sub));
}

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
