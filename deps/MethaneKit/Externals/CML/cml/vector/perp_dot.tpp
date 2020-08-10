/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_VECTOR_PERP_DOT_TPP
#error "vector/perp_dot.tpp not included correctly"
#endif

#include <cml/vector/readable_vector.h>
#include <cml/vector/size_checking.h>

namespace cml {

template<class Sub1, class Sub2> inline auto perp_dot(
  const readable_vector<Sub1>& left, const readable_vector<Sub2>& right
  ) -> perp_dot_promote_t<Sub1,Sub2>
{
  typedef perp_dot_promote_t<Sub1,Sub2> result_type;
  cml::check_size(left, cml::int_c<2>());
  cml::check_size(right, cml::int_c<2>());
  return result_type(left[0]*right[1] - left[1]*right[0]);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
