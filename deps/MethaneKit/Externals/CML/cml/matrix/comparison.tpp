/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_COMPARISON_TPP
#error "matrix/comparison.tpp not included correctly"
#endif

#include <cml/matrix/size_checking.h>

namespace cml {

template<class Sub1, class Sub2> inline bool operator==(
  const readable_matrix<Sub1>& left, const readable_matrix<Sub2>& right
  )
{
  /* Possibly equal only if the same dimensions: */
  if(left.size() != right.size()) return false;
  for(int i = 0; i < left.rows(); i ++) {
    for(int j = 0; j < left.cols(); j ++) {
      /**/ if(left(i, j) < right(i, j)) return false;	// Strictly less.
      else if(right(i, j) < left(i, j)) return false;	// Strictly greater.
      else continue;				        // Possibly equal.
    }
  }
  return true;
}

template<class Sub1, class Sub2> inline bool operator!=(
  const readable_matrix<Sub1>& left, const readable_matrix<Sub2>& right
  )
{
  return !(left == right);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
