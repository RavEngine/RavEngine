/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_UTIL_MATRIX_PRINT_TPP
#error "util/matrix_print.tpp not included correctly"
#endif

#include <iostream>
#include <cml/matrix/readable_matrix.h>

namespace cml {

template<class DT> inline std::ostream&
operator<<(std::ostream& os, const readable_matrix<DT>& M)
{
  for(int i = 0; i < M.rows(); ++i) {
    os << "[";
    for(int j = 0; j < M.cols(); ++j) os << " " << M(i,j);
    os << " ]";
    if (i != M.rows()-1) os << std::endl;
  }
  return os;
}

} // namespace cml


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
