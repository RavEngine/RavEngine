/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_detail_check_or_resize_h
#define	cml_matrix_detail_check_or_resize_h

#include <cml/matrix/writable_matrix.h>
#include <cml/matrix/size_checking.h>

namespace cml {
namespace detail {

/** Ensure non-resizable matrix @c left is the same size as @c right. */
template<class Sub, class Other> inline void
check_or_resize(const readable_matrix<Sub>& left, const Other& right)
{
  cml::check_same_size(left, right);
}

/** Ensure resizable matrix @c left is the same size as @c right. */
template<class Sub1, class Sub2> inline auto
check_or_resize(
  writable_matrix<Sub1>& left, const readable_matrix<Sub2>& right
  )
-> decltype(left.actual().resize(0,0), void())
{
  left.actual().resize(right.rows(),right.cols());
}

/** Ensure resizable matrix @c left is the same size as array @c right. */
template<class Sub1, class Other, int Rows, int Cols> inline auto
check_or_resize(
  writable_matrix<Sub1>& left, Other const (&)[Rows][Cols]
  )
-> decltype(left.actual().resize(0,0), void())
{
  left.actual().resize(Rows, Cols);
}


/* check_or_resize for a read-only matrix left and constant size RxC that
 * just forwards to check_size.
 */
template<class Sub, int R, int C> inline void
check_or_resize(const readable_matrix<Sub>& sub, int_c<R>, int_c<C>)
{
  cml::check_size(sub, int_c<R>(), int_c<C>());
}

/* check_or_resize for a read-only matrix left and run-time size RxC that
 * just forwards to check_size.
 */
template<class Sub> inline void
check_or_resize(const readable_matrix<Sub>& sub, int R, int C)
{
  cml::check_size(sub, R, C);
}

/* check_or_resize for a resizable matrix left and compile-time size that
 * resizes the matrix to RxC.
 */
template<class Sub, int R, int C> inline auto
check_or_resize(writable_matrix<Sub>& sub, int_c<R>, int_c<C>)
-> decltype(sub.actual().resize(0,0), void())
{
  sub.actual().resize(R,C);
}

/* check_or_resize for a resizable matrix left and run-time size that
 * resizes the matrix to RxC.
 */
template<class Sub> inline auto
check_or_resize(writable_matrix<Sub>& sub, int R, int C)
-> decltype(sub.actual().resize(0,0), void())
{
  sub.actual().resize(R,C);
}

} // namespace detail
} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
