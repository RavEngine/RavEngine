/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_perp_dot_h
#define	cml_vector_perp_dot_h

#include <cml/scalar/promotion.h>
#include <cml/vector/traits.h>

namespace cml {

/** Convenience alias to determine the scalar type to return from
 * cml::perp_dot.
 */
template<class Sub1, class Sub2>
  using perp_dot_promote_t = value_type_trait_promote_t<Sub1, Sub2>;

/** Compute the "cross-product" of two 2D vectors, and return the scalar
 * result.
 *
 * @note Currently, the result is computed immediately, even if it appears
 * in an expression.
 *
 * @throws vector_size_error at run-time if left or right is
 * dynamically-sized and is not a 2D vector.  The size is checked at
 * compile time for fixed-sized expressions.
 */
template<class Sub1, class Sub2> auto perp_dot(
  const readable_vector<Sub1>& left, const readable_vector<Sub2>& right)
  -> perp_dot_promote_t<Sub1,Sub2>;

} // namespace cml

#define __CML_VECTOR_PERP_DOT_TPP
#include <cml/vector/perp_dot.tpp>
#undef __CML_VECTOR_PERP_DOT_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
