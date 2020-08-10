/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_unary_ops_h
#define	cml_vector_unary_ops_h

#include <cml/scalar/unary_ops.h>
#include <cml/vector/unary_node.h>

namespace cml {

/** Helper function to generate a vector_unary_node from a vector type
 * (i.e. derived from readable_vector<>).
 */
template<class Op, class Sub, enable_if_vector_t<Sub>* = nullptr>
inline auto make_vector_unary_node(Sub&& sub)
-> vector_unary_node<actual_operand_type_of_t<decltype(sub)>, Op>
{
  static_assert(std::is_same<
    decltype(sub), decltype(std::forward<Sub>(sub))>::value,
    "internal error: unexpected expression type");

  /* Deduce the operand type of the subexpression (&, const&, &&): */
  typedef actual_operand_type_of_t<decltype(sub)> sub_type;
  return vector_unary_node<sub_type, Op>((sub_type) sub);
}

template<class Sub, enable_if_vector_t<Sub>* = nullptr>
inline auto operator-(Sub&& sub)
-> decltype(make_vector_unary_node<unary_minus_t<Sub>>(std::forward<Sub>(sub)))
{
  return make_vector_unary_node<unary_minus_t<Sub>>(std::forward<Sub>(sub));
}

template<class Sub, enable_if_vector_t<Sub>* = nullptr>
inline auto operator+(Sub&& sub)
-> decltype(make_vector_unary_node<unary_plus_t<Sub>>(std::forward<Sub>(sub)))
{
  return make_vector_unary_node<unary_plus_t<Sub>>(std::forward<Sub>(sub));
}

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
