/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_DETERMINANT_TPP
#error "matrix/determinant.tpp not included correctly"
#endif

#include <cml/matrix/readable_matrix.h>

namespace cml {

template<class Sub> inline auto
determinant(const readable_matrix<Sub>& M)
-> value_type_trait_of_t<Sub>
{
  return M.determinant();
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
