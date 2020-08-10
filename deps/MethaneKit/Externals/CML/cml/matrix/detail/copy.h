/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_detail_copy_h
#define	cml_matrix_detail_copy_h

#include <cml/matrix/detail/get.h>

namespace cml {
namespace detail {

/** Assign @c left from the elements of @c right, where @c left is assumed
 * to have a row-major layout.
 */
template<class Sub, class Other> inline void copy(
  writable_matrix<Sub>& left, const Other& right, row_major
  )
{
  for(int i = 0; i < left.rows(); ++ i)
    for(int j = 0; j < left.cols(); ++ j)
      left.put(i,j, get(right, i,j));
}

/** Assign @c left from the elements of @c right, where @c left is assumed
 * to have a column-major layout.
 */
template<class Sub, class Other> inline void copy(
  writable_matrix<Sub>& left, const Other& right, col_major
  )
{
  for(int j = 0; j < left.cols(); ++ j)
    for(int i = 0; i < left.rows(); ++ i)
      left.put(i,j, get(right, i,j));
}

} // namespace detail
} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
