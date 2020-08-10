/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_VECTOR_FUNCTIONS_TPP
#error "vector/functions.tpp not included correctly"
#endif

#include <cml/vector/readable_vector.h>

namespace cml {

template<class DT> inline auto
length_squared(const readable_vector<DT>& v) -> value_type_trait_of_t<DT>
{
  return v.length_squared();
}

template<class DT> inline auto
length(const readable_vector<DT>& v) -> value_type_trait_of_t<DT>
{
  return v.length();
}

template<class DT> inline auto
normalize(const readable_vector<DT>& v) -> temporary_of_t<DT>
{
  return v.normalize();
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
