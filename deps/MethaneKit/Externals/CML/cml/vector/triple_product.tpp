/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_VECTOR_TRIPLE_PRODUCT_TPP
#error "vector/triple_product.tpp not included correctly"
#endif

#include <cml/vector/dot.h>
#include <cml/vector/cross.h>

namespace cml {

template<class Sub1, class Sub2, class Sub3> inline auto
triple_product(const readable_vector<Sub1>& a,
  const readable_vector<Sub2>& b, const readable_vector<Sub3>& c
  ) -> triple_product_promote_t<Sub1,Sub2,Sub3>
{
  cml::check_size(a, cml::int_c<3>());
  cml::check_size(b, cml::int_c<3>());
  cml::check_size(c, cml::int_c<3>());
  return cml::dot(a, cross(b,c));
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
