/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_scalar_ops_h
#define	cml_vector_scalar_ops_h

#include <cml/common/mpl/enable_if_arithmetic.h>
#include <cml/scalar/binary_ops.h>
#include <cml/vector/scalar_node.h>

namespace cml {

/** Helper function to generate a vector_scalar_node from a vector type
 * (i.e. derived from readable_vector<>) and a scalar type.
 */
template<class Op, class Sub, class Scalar,
  enable_if_vector_t<Sub>* = nullptr,
  enable_if_arithmetic_t<cml::unqualified_type_t<Scalar>>* = nullptr
> inline auto
make_vector_scalar_node(Sub&& sub, Scalar&& v)
-> vector_scalar_node<
actual_operand_type_of_t<decltype(sub)>,
actual_operand_type_of_t<decltype(v)>,
Op
>
{
  static_assert(std::is_same<
    decltype(sub), decltype(std::forward<Sub>(sub))>::value,
    "internal error: unexpected expression type (sub)");
  static_assert(std::is_same<
    decltype(v), decltype(std::forward<Scalar>(v))>::value,
    "internal error: unexpected expression type (v)");

  /* Deduce the operand types of the scalar and the subexpression (&,
   * const&, &&):
   */
  typedef actual_operand_type_of_t<decltype(sub)> sub_type;
  typedef actual_operand_type_of_t<decltype(v)> scalar_type;
  return vector_scalar_node<sub_type, scalar_type, Op>(
    (sub_type) sub, (scalar_type) v);
}

template<class Sub, class Scalar,
  enable_if_vector_t<Sub>* = nullptr,
  enable_if_arithmetic_t<cml::unqualified_type_t<Scalar>>* = nullptr
>
inline auto operator*(Sub&& sub, Scalar&& v)
-> decltype(make_vector_scalar_node<binary_multiply_t<Sub,Scalar>>(
    std::forward<Sub>(sub), std::forward<Scalar>(v)))
{
  return make_vector_scalar_node<binary_multiply_t<Sub,Scalar>>(
    std::forward<Sub>(sub), std::forward<Scalar>(v));
}

template<class Scalar, class Sub,
  enable_if_arithmetic_t<cml::unqualified_type_t<Scalar>>* = nullptr,
  enable_if_vector_t<Sub>* = nullptr
>
inline auto operator*(Scalar&& v, Sub&& sub)
-> decltype(make_vector_scalar_node<binary_multiply_t<Sub,Scalar>>(
    std::forward<Sub>(sub), std::forward<Scalar>(v)))
{
  return make_vector_scalar_node<binary_multiply_t<Sub,Scalar>>(
    std::forward<Sub>(sub), std::forward<Scalar>(v));
}

template<class Sub, class Scalar,
  enable_if_vector_t<Sub>* = nullptr,
  enable_if_arithmetic_t<cml::unqualified_type_t<Scalar>>* = nullptr
>
inline auto operator/(Sub&& sub, Scalar&& v)
-> decltype(make_vector_scalar_node<binary_divide_t<Sub,Scalar>>(
    std::forward<Sub>(sub), std::forward<Scalar>(v)))
{
  return make_vector_scalar_node<binary_divide_t<Sub,Scalar>>(
    std::forward<Sub>(sub), std::forward<Scalar>(v));
}

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
