/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_triple_product_h
#define	cml_vector_triple_product_h

#include <cml/scalar/promotion.h>
#include <cml/vector/traits.h>

namespace cml {

/** Convenience alias to determine the scalar type to return from
 * cml::triple_product.
 */
template<class Sub1, class Sub2, class Sub3>
  using triple_product_promote_t = value_type_trait_promote_t<Sub1,Sub2,Sub3>;

/** Compute the scalar triple product ("box product") of 3 3D vectors, and
 * return the scalar result.
 *
 * @note Currently, the result is computed immediately, even if it appears
 * as a term in an expression.
 *
 * @throws vector_size_error at run-time if a, b, or c is dynamically-sized
 * and is not a 3D vector.  The size is checked at compile time for
 * fixed-sized expressions.
 */
template<class Sub1, class Sub2, class Sub3> auto triple_product(
  const readable_vector<Sub1>& a, const readable_vector<Sub2>& b,
  const readable_vector<Sub3>& c) -> triple_product_promote_t<Sub1,Sub2,Sub3>;

} // namespace cml

#define __CML_VECTOR_TRIPLE_PRODUCT_TPP
#include <cml/vector/triple_product.tpp>
#undef __CML_VECTOR_TRIPLE_PRODUCT_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
