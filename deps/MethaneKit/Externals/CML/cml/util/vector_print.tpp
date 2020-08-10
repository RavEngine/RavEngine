/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_UTIL_VECTOR_PRINT_TPP
#error "util/vector_print.tpp not included correctly"
#endif

#include <iostream>
#include <cml/vector/readable_vector.h>

namespace cml {

template<class DT> inline std::ostream&
operator<<(std::ostream& os, const readable_vector<DT>& v)
{
  os << v[0];
  for (int i = 1; i < v.size(); ++i) os << " " << v[i];
  return os;
}

} // namespace cml


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
