/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_util_vector_print_h
#define	cml_util_vector_print_h

#include <iosfwd>

namespace cml {

/* Forward declarations: */
template<class DerivedT> class readable_vector;

/** Output a vector to a std::ostream. */
template<class DerivedT> std::ostream& operator<<(
  std::ostream& os, const readable_vector<DerivedT>& v);

} // namespace cml


#define __CML_UTIL_VECTOR_PRINT_TPP
#include <cml/util/vector_print.tpp>
#undef __CML_UTIL_VECTOR_PRINT_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
