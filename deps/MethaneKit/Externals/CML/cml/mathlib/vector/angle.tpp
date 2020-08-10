/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATHLIB_VECTOR_ANGLE_TPP
#error "mathlib/vector/angle.tpp not included correctly"
#endif

#include <cml/vector/dot.h>
#include <cml/vector/perp_dot.h>
#include <cml/vector/cross.h>

namespace cml {

template<class Sub1, class Sub2> inline auto
signed_angle_2D(
  const readable_vector<Sub1>& v1,
  const readable_vector<Sub2>& v2
  )
-> value_type_trait_promote_t<Sub1,Sub2>
{
  typedef value_type_trait_promote_t<Sub1,Sub2>		value_type;
  typedef scalar_traits<value_type>			value_traits;
  return value_traits::atan2(perp_dot(v1,v2), dot(v1, v2));
}

template<class Sub1, class Sub2> inline auto
unsigned_angle_2D(
  const readable_vector<Sub1>& v1,
  const readable_vector<Sub2>& v2
  )
-> value_type_trait_promote_t<Sub1,Sub2>
{
  typedef value_type_trait_promote_t<Sub1,Sub2>		value_type;
  typedef scalar_traits<value_type>			value_traits;
  return value_traits::fabs(signed_angle_2D(v1, v2));
}

template<class Sub1, class Sub2, class Sub3> inline auto
signed_angle(
  const readable_vector<Sub1>& v1,
  const readable_vector<Sub2>& v2,
  const readable_vector<Sub3>& reference
  )
-> value_type_trait_promote_t<Sub1,Sub2,Sub3>
{
  typedef value_type_trait_promote_t<Sub1,Sub2,Sub3>	value_type;
  typedef scalar_traits<value_type>			value_traits;

  auto c = cross(v1, v2);
  auto angle = value_traits::atan2(c.length(), dot(v1, v2));
  return (dot(c, reference) < value_type(0)) ? -angle : angle;
}

template<class Sub1, class Sub2> inline auto
unsigned_angle(
  const readable_vector<Sub1>& v1,
  const readable_vector<Sub2>& v2
  )
-> value_type_trait_promote_t<Sub1,Sub2>
{
  typedef value_type_trait_promote_t<Sub1,Sub2>		value_type;
  typedef scalar_traits<value_type>			value_traits;
  return value_traits::atan2(cross(v1,v2).length(), dot(v1, v2));
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
