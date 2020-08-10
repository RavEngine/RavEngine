/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_QUATERNION_DOT_TPP
#error "quaternion/dot.tpp not included correctly"
#endif

#include <cml/quaternion/readable_quaternion.h>
#include <cml/quaternion/size_checking.h>

namespace cml {

template<class Sub1, class Sub2> inline auto dot(
  const readable_quaternion<Sub1>& left, const readable_quaternion<Sub2>& right
  )
-> value_type_trait_promote_t<Sub1,Sub2>
{
  typedef value_type_trait_promote_t<Sub1,Sub2>		value_type;

  static_assert(std::is_same<order_type_trait_of_t<Sub1>,
    order_type_trait_of_t<Sub2>>::value, "mismatched quaternion order types");
  return value_type(
    left.get(0)*right.get(0) +
    left.get(1)*right.get(1) +
    left.get(2)*right.get(2) +
    left.get(3)*right.get(3))
    ;
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
