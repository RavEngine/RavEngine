/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_detail_transpose_h
#define	cml_matrix_detail_transpose_h

#include <cml/common/mpl/enable_if_reshapeable.h>
#include <cml/matrix/writable_matrix.h>
#include <cml/matrix/temporary.h>
#include <cml/matrix/transpose.h>

namespace cml {
namespace detail {

/** Transpose a fixed-size square matrix. */
template<class Sub> inline void
transpose(writable_matrix<Sub>& M, fixed_size_tag)
{
  static const int rows = array_rows_of_c<Sub>::value;
  static const int cols = array_cols_of_c<Sub>::value;
  static_assert(rows == cols, "non-square fixed-size matrix");
  for(int i = 1; i < rows; ++ i)
    for(int j = 0; j < i; ++ j)
      std::swap(M.get(i,j), M.get(j,i));
}

/** Transpose a resizable matrix using a temporary. */
template<class Sub, enable_if_reshapeable_t<Sub>* = nullptr> inline void
transpose(writable_matrix<Sub>& M, dynamic_size_tag)
{
  temporary_of_t<Sub> T(M);
  M = cml::transpose(T);
}

} // namespace detail
} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
