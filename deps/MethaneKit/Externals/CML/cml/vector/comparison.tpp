/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_VECTOR_COMPARISON_TPP
#error "vector/comparison.tpp not included correctly"
#endif

#include <cml/vector/size_checking.h>

namespace cml {

template<class Sub1, class Sub2> inline bool operator<(
  const readable_vector<Sub1>& left, const readable_vector<Sub2>& right
  )
{
  int n = std::min(left.size(), right.size());
  for(int i = 0; i < n; i ++) {
    /**/ if(left[i] < right[i]) return true;	// Strictly less.
    else if(right[i] < left[i]) return false;	// Strictly greater.
    else continue;				// Possibly equal.
  }

  /* Equal only if the same length: */
  return left.size() < right.size();
}

template<class Sub1, class Sub2> inline bool operator>(
  const readable_vector<Sub1>& left, const readable_vector<Sub2>& right
  )
{
  int n = std::min(left.size(), right.size());
  for(int i = 0; i < n; i ++) {
    /**/ if(left[i] < right[i]) return false;	// Strictly less.
    else if(right[i] < left[i]) return true;	// Strictly greater.
    else continue;				// Possibly equal.
  }

  /* Equal only if the same length: */
  return left.size() > right.size();
}

template<class Sub1, class Sub2> inline bool operator==(
  const readable_vector<Sub1>& left, const readable_vector<Sub2>& right
  )
{
  /* Possibly equal only if the same length: */
  if(left.size() != right.size()) return false;
  for(int i = 0; i < left.size(); i ++) {
    /**/ if(left[i] < right[i]) return false;	// Strictly less.
    else if(right[i] < left[i]) return false;	// Strictly greater.
    else continue;				// Possibly equal.
  }
  return true;
}

template<class Sub1, class Sub2> inline bool operator<=(
  const readable_vector<Sub1>& left, const readable_vector<Sub2>& right
  )
{
  return !(left > right);
}

template<class Sub1, class Sub2> inline bool operator>=(
  const readable_vector<Sub1>& left, const readable_vector<Sub2>& right
  )
{
  return !(left < right);
}

template<class Sub1, class Sub2> inline bool operator!=(
  const readable_vector<Sub1>& left, const readable_vector<Sub2>& right
  )
{
  return !(left == right);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
