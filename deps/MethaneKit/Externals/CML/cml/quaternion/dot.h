/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_dot_h
#define	cml_quaternion_dot_h

#include <cml/scalar/promotion.h>
#include <cml/quaternion/traits.h>

namespace cml {

/** Compute the dot-product of two quaternions.
 *
 * @note Currently, the result is computed immediately, even if it appears
 * as a term in an expression.
 *
 * @note Compilation will fail if @c Sub1 and @c Sub2 have different
 * quaternion orders.
 */
template<class Sub1, class Sub2> auto dot(
  const readable_quaternion<Sub1>& left, const readable_quaternion<Sub2>& right
  )
-> value_type_trait_promote_t<Sub1,Sub2>;

} // namespace cml

#define __CML_QUATERNION_DOT_TPP
#include <cml/quaternion/dot.tpp>
#undef __CML_QUATERNION_DOT_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
