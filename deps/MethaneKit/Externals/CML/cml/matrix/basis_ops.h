/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_basis_ops_h
#define	cml_matrix_basis_ops_h

#include <cml/matrix/type_util.h>
#include <cml/matrix/basis_node.h>

namespace cml {

template<class Sub, enable_if_matrix_t<Sub>* = nullptr>
inline auto basis(Sub&& sub, int i)
-> matrix_basis_node<actual_operand_type_of_t<decltype(sub)>, -1>
{
  static_assert(std::is_same<
    decltype(sub), decltype(std::forward<Sub>(sub))>::value,
    "internal error: unexpected expression type");

  /* Deduce the operand type of the subexpression (&, const&, &&): */
  typedef actual_operand_type_of_t<decltype(sub)> sub_type;
  return matrix_basis_node<sub_type, -1>((sub_type) sub, i);
}

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
