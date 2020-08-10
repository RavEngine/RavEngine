/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_detail_get_h
#define	cml_matrix_detail_get_h

#include <cml/matrix/writable_matrix.h>

namespace cml {
namespace detail {

/** Helper to return the passed-in value in response to a matrix index @c
 * (i,j).
 */
template<class Other>
inline auto get(const Other& v, int, int) -> const Other&
{
  return v;
}

/** Helper to return element @c (i,j) of @c array. */
template<class Other, int Rows, int Cols> inline const Other&
get(Other const (&array)[Rows][Cols], int i, int j)
{
  return array[i][j];
}

/** Helper to return element @c (i,j) of @c sub. */
template<class Sub> inline auto get(
  const readable_matrix<Sub>& sub, int i, int j
  ) -> typename matrix_traits<Sub>::immutable_value
{
  return sub.get(i,j);
}

} // namespace detail
} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
