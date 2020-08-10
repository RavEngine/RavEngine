/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_scalar_unary_ops_h
#define	cml_scalar_unary_ops_h

#include <cml/common/type_util.h>
#include <cml/scalar/traits.h>

namespace cml {
namespace op {

/** Unary minus (negation). */
template<class Scalar> struct unary_minus {
  typedef value_type_trait_of_t<Scalar> value_type;
  typedef decltype(- value_type()) result_type;
  result_type apply(const value_type& v) const { return - v; }
};

/** Unary plus. */
template<class Scalar> struct unary_plus {
  typedef value_type_trait_of_t<Scalar> value_type;
  typedef decltype(+ value_type()) result_type;
  result_type apply(const value_type& v) const { return + v; }
};

} // namespace op

/** Convenience alias to create unary_minus from the value_type trait of
 * @c Sub as an unqualified type.
 */
template<class Sub> using unary_minus_t
  = op::unary_minus<value_type_trait_of_t<actual_type_of_t<Sub>>>;

/** Convenience alias to create unary_plus from the value_type trait of
 * @c Sub as an unqualified type.
 */
template<class Sub> using unary_plus_t
  = op::unary_plus<value_type_trait_of_t<actual_type_of_t<Sub>>>;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
