/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_VECTOR_DOT_TPP
#error "vector/dot.tpp not included correctly"
#endif

#include <cml/vector/readable_vector.h>
#include <cml/vector/size_checking.h>

namespace cml {

template<class Sub1, class Sub2> inline auto
dot(const readable_vector<Sub1>& left, const readable_vector<Sub2>& right)
-> value_type_trait_promote_t<Sub1,Sub2>
{
  typedef value_type_trait_promote_t<Sub1,Sub2> result_type;
  cml::check_minimum_size(left, cml::int_c<1>());
  cml::check_minimum_size(right, cml::int_c<1>());
  cml::check_same_size(left, right);
  result_type accum = result_type(left.get(0)*right.get(0));
  for(int i = 1; i < left.size(); ++ i)
    accum += result_type(left.get(i)*right.get(i));
  return accum;
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
