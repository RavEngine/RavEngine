/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATHLIB_VECTOR_ROTATION_TPP
#error "mathlib/vector/rotation.tpp not included correctly"
#endif

#include <cml/common/mpl/are_convertible.h>
#include <cml/vector/dot.h>
#include <cml/vector/cross.h>

namespace cml {

template<class Sub1, class Sub2, class E> inline auto
rotate_vector(
  const readable_vector<Sub1>& v, const readable_vector<Sub2>& n,
  const E& angle
  ) -> vector_promote_t<Sub1,Sub2>
{
  static_assert(cml::are_convertible<
    value_type_trait_of_t<Sub1>, value_type_trait_of_t<Sub2>, E>::value,
    "incompatible scalar types");

  typedef scalar_traits<E>				angle_traits;

  cml::check_size(v, cml::int_c<3>());
  cml::check_size(n, cml::int_c<3>());

  auto parallel = dot(v,n)*n;
  auto sin_angle = angle_traits::sin(angle);
  auto cos_angle = angle_traits::cos(angle);
  return cos_angle*(v - parallel) + sin_angle*cross(n,v) + parallel;
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
