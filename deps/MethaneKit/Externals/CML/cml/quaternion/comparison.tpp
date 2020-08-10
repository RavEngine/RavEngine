/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_QUATERNION_COMPARISON_TPP
#error "quaternion/comparison.tpp not included correctly"
#endif

#include <type_traits>

namespace cml {

template<class Sub1, class Sub2> inline bool operator<(
  const readable_quaternion<Sub1>& left, const readable_quaternion<Sub2>& right
  )
{
  static_assert(std::is_same<
    order_type_trait_of_t<Sub1>, order_type_trait_of_t<Sub2>>::value,
    "cannot compare quaternions with different orders");
  for(int i = 0; i < 4; i ++) {
    /**/ if(left[i] < right[i]) return true;	// Strictly less.
    else if(left[i] > right[i]) return false;	// Strictly greater.
    else continue;				// Equal.
  }

  /* Equal. */
  return false;
}

template<class Sub1, class Sub2> inline bool operator>(
  const readable_quaternion<Sub1>& left, const readable_quaternion<Sub2>& right
  )
{
  static_assert(std::is_same<
    order_type_trait_of_t<Sub1>, order_type_trait_of_t<Sub2>>::value,
    "cannot compare quaternions with different orders");
  for(int i = 0; i < 4; i ++) {
    /**/ if(left[i] > right[i]) return true;	// Strictly greater.
    else if(left[i] < right[i]) return false;	// Strictly less.
    else continue;				// Equal.
  }

  /* Equal. */
  return false;
}

template<class Sub1, class Sub2> inline bool operator==(
  const readable_quaternion<Sub1>& left, const readable_quaternion<Sub2>& right
  )
{
  static_assert(std::is_same<
    order_type_trait_of_t<Sub1>, order_type_trait_of_t<Sub2>>::value,
    "cannot compare quaternions with different orders");
  for(int i = 0; i < 4; i ++) {
    if(!(left[i] == right[i])) return false;	// Not equal.
  }
  return true;
}

template<class Sub1, class Sub2> inline bool operator<=(
  const readable_quaternion<Sub1>& left, const readable_quaternion<Sub2>& right
  )
{
  return !(left > right);
}

template<class Sub1, class Sub2> inline bool operator>=(
  const readable_quaternion<Sub1>& left, const readable_quaternion<Sub2>& right
  )
{
  return !(left < right);
}

template<class Sub1, class Sub2> inline bool operator!=(
  const readable_quaternion<Sub1>& left, const readable_quaternion<Sub2>& right
  )
{
  return !(left == right);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
