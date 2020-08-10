/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_util_matrix_print_h
#define	cml_util_matrix_print_h

#include <iosfwd>

namespace cml {

/* Forward declarations: */
template<class DerivedT> class readable_matrix;

/** Output a matrix to a std::ostream. */
template<class DerivedT> std::ostream& operator<<(
  std::ostream& os, const readable_matrix<DerivedT>& v);

} // namespace cml


#define __CML_UTIL_MATRIX_PRINT_TPP
#include <cml/util/matrix_print.tpp>
#undef __CML_UTIL_MATRIX_PRINT_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
