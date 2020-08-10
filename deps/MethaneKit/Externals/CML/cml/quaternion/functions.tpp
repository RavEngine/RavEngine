/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_QUATERNION_FUNCTIONS_TPP
#error "quaternion/functions.tpp not included correctly"
#endif

#include <cml/quaternion/readable_quaternion.h>

namespace cml {

template<class DT> inline auto
real(const readable_quaternion<DT>& q)
-> value_type_trait_of_t<DT>
{
  return q.real();
}

template<class DT> inline auto
length_squared(const readable_quaternion<DT>& q)
-> value_type_trait_of_t<DT>
{
  return q.length_squared();
}

template<class DT> inline auto
length(const readable_quaternion<DT>& q)
-> value_type_trait_of_t<DT>
{
  return q.length();
}

template<class DT> inline auto
norm(const readable_quaternion<DT>& q)
-> value_type_trait_of_t<DT>
{
  return q.norm();
}

template<class DerivedT> inline auto
normalize(const readable_quaternion<DerivedT>& q)
-> temporary_of_t<DerivedT>
{
  return q.normalize();
}

template<class DerivedT> inline auto
identity(const readable_quaternion<DerivedT>& q)
-> temporary_of_t<DerivedT>
{
  temporary_of_t<DerivedT> result(q);
  result.identity();
  return result;
}

template<class DerivedT> inline auto
log(const readable_quaternion<DerivedT>& q)
-> temporary_of_t<DerivedT>
{
  temporary_of_t<DerivedT> result(q);
  result.log();
  return result;
}

template<class DerivedT> inline auto
exp(const readable_quaternion<DerivedT>& q)
-> temporary_of_t<DerivedT>
{
  temporary_of_t<DerivedT> result(q);
  result.exp();
  return result;
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
