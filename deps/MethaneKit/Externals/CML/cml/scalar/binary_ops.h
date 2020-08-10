/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_scalar_binary_ops_h
#define	cml_scalar_binary_ops_h

#include <cml/scalar/traits.h>
#include <cml/scalar/promotion.h>

namespace cml {
namespace op {

#define __cml_binary_op(_name_, _op_)					\
template<class Scalar1, class Scalar2> struct _name_ {			\
  typedef value_type_trait_promote_t<Scalar1,Scalar2> result_type;	\
  result_type apply(const Scalar1& a, const Scalar2& b) const {		\
    return result_type(a _op_ b); }					\
}

/** Binary minus (subtraction). */
__cml_binary_op(binary_minus, -);

/** Binary plus (addition). */
__cml_binary_op(binary_plus, +);

/** Binary multiply. */
__cml_binary_op(binary_multiply, *);

/** Binary divide. */
__cml_binary_op(binary_divide, /);

#undef __cml_binary_op

} // namespace op

#define __cml_binary_op_alias(_name_)					\
template<class Sub1, class Sub2>					\
  using _name_ ## _t = op:: _name_ <					\
    value_type_trait_of_t<actual_type_of_t<Sub1>>,			\
    value_type_trait_of_t<actual_type_of_t<Sub2>>>

/** Convenience alias to create binary_minus from the value_type traits of
 * @c Sub1 and @c Sub2 as unqualified types.
 */
__cml_binary_op_alias(binary_minus);

/** Convenience alias to create binary_plus from the value_type traits of
 * @c Sub1 and @c Sub2 as unqualified types.
 */
__cml_binary_op_alias(binary_plus);

/** Convenience alias to create binary_multiply from the value_type traits
 * of @c Sub1 and @c Sub2 as unqualified types.
 */
__cml_binary_op_alias(binary_multiply);

/** Convenience alias to create binary_divide from the value_type traits
 * of @c Sub1 and @c Sub2 as unqualified types.
 */
__cml_binary_op_alias(binary_divide);

#undef __cml_binary_op_alias
} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
