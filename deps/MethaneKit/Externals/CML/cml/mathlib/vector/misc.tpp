/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATHLIB_VECTOR_MISC_TPP
#error "mathlib/vector/misc.tpp not included correctly"
#endif

#include <cml/vector/ops.h>
#include <cml/vector/dot.h>

namespace cml {

template<class Sub1, class Sub2> inline auto
project_to_vector(
  const readable_vector<Sub1>& u, const readable_vector<Sub2>& v
  )
-> vector_promote_t<Sub1, Sub2>
{
  typedef vector_promote_t<Sub1, Sub2>			result_type;
  return result_type((dot(u,v) / length_squared(v)) * v);
}  

template<class Sub1, class Sub2> inline auto
project_to_hplane(
  const readable_vector<Sub1>& v, const readable_vector<Sub2>& n
  )
-> vector_promote_t<Sub1, Sub2>
{
  typedef vector_promote_t<Sub1, Sub2>			result_type;
  return result_type(v - dot(v,n)*n);
}

template<class Sub> inline auto
perp(const readable_vector<Sub>& v) -> temporary_of_t<Sub>
{
  cml::check_size(v, cml::int_c<2>());
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
  return temporary_of_t<Sub>(- v[1], v[0]);
#else
  return { - v[1], v[0] };
#endif
}

template<class Sub1, class Sub2> inline auto
manhattan_distance(
  const readable_vector<Sub1>& v1, const readable_vector<Sub2>& v2
  )
-> value_type_trait_promote_t<Sub1, Sub2>
{
  typedef value_type_trait_promote_t<Sub1, Sub2>	value_type;
  typedef scalar_traits<value_type>			value_traits;

  cml::check_minimum_size(v1, cml::int_c<1>());
  cml::check_minimum_size(v2, cml::int_c<1>());
  cml::check_same_size(v1,v2);

  auto fabs = &value_traits::fabs;
  auto sum = fabs(value_type(v1[0] - v2[0]));
  for(int i = 1; i < v1.size(); ++ i)
    sum += fabs(value_type(v1[i] - v2[i]));
  return sum;
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
