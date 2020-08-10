/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_comparison_h
#define	cml_matrix_comparison_h

#include <cml/matrix/readable_matrix.h>

namespace cml {

/** Returns true if the elements of @c left are all equal to the elements
 * of @c right.
 */
template<class Sub1, class Sub2> bool operator==(
  const readable_matrix<Sub1>& left, const readable_matrix<Sub2>& right);

/** Returns true if some element of @c left is not equal to the same element
 * of @c right.
 */
template<class Sub1, class Sub2> bool operator!=(
  const readable_matrix<Sub1>& left, const readable_matrix<Sub2>& right);

} // namespace cml

#define __CML_MATRIX_COMPARISON_TPP
#include <cml/matrix/comparison.tpp>
#undef __CML_MATRIX_COMPARISON_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
