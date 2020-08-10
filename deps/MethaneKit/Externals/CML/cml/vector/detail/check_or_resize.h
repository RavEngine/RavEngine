/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 *
 * @note This file must be included after cml/vector/writable_vector.h.
 */

#pragma once

#ifndef	cml_vector_detail_check_or_resize_h
#define	cml_vector_detail_check_or_resize_h

#include <cml/common/mpl/int_c.h>
#include <cml/vector/size_checking.h>
#include <cml/vector/detail/combined_size_of.h>

namespace cml {
namespace detail {

/* check_or_resize for a read-only vector, left, that just forwards to
 * check_same_size.
 */
template<class Sub, class Other> inline void
check_or_resize(const readable_vector<Sub>& left, const Other& right)
{
  cml::check_same_size(left, right);
}

/* check_or_resize for a resizable vector, left, that resizes the vector to
 * ensure it has the same size as right.
 */
template<class Sub, class Other> inline auto
check_or_resize(writable_vector<Sub>& left, const Other& right)
-> decltype(left.actual().resize(0), void())
{
  left.actual().resize(cml::array_size_of(right));
}

/* check_or_resize for a read-only vector left and constant size N that
 * just forwards to check_size.
 */
template<class Sub, int N> inline void
check_or_resize(const readable_vector<Sub>& sub, int_c<N>)
{
  cml::check_size(sub, int_c<N>());
}

/* check_or_resize for a read-only vector left and run-time size N that
 * just forwards to check_size.
 */
template<class Sub> inline void
check_or_resize(const readable_vector<Sub>& sub, int N)
{
  cml::check_size(sub, N);
}

/* check_or_resize for a resizable vector left and compile-time size N that
 * resizes the vector to N.
 */
template<class Sub, int N> inline auto
check_or_resize(writable_vector<Sub>& sub, int_c<N>)
-> decltype(sub.actual().resize(0), void())
{
  sub.actual().resize(N);
}

/* check_or_resize for a resizable vector left and run-time size N that
 * resizes the vector to N.
 */
template<class Sub> inline auto
check_or_resize(writable_vector<Sub>& sub, int N)
-> decltype(sub.actual().resize(0), void())
{
  sub.actual().resize(N);
}


/* check_or_resize for a read-only vector, left, that just forwards to
 * check_same_size.
 */
template<class Sub, class Other> inline void
check_or_resize(
  const readable_vector<Sub>& left, const readable_vector<Other>& right
  )
{
  cml::check_same_size(left, right);
}

/* check_or_resize for a resizable vector, left, that resizes the vector to
 * ensure it has the same size as right.
 */
template<class Sub, class Other> inline auto
check_or_resize(
  writable_vector<Sub>& left, const readable_vector<Other>& right
  )
-> decltype(left.actual().resize(0), void())
{
  left.actual().resize(cml::array_size_of(right));
}

/* check_or_resize for a read-only vector that verifies the size is
 * other.size() + sizeof(eN):
 */
template<class Sub, class Other, class... Elements> inline void
check_or_resize(const readable_vector<Sub>& sub,
  const readable_vector<Other>& other, const Elements&... eN
  )
{
  cml::check_size(sub, combined_size_of(other, eN...));
}

/* check_or_resize for a resizable vector that resizes the vector to
 * other.size() + sizeof(eN):
 */
template<class Sub, class Other, class... Elements> inline auto
check_or_resize(writable_vector<Sub>& sub,
  const readable_vector<Other>& other, const Elements&... eN
  )
-> decltype(sub.actual().resize(0), void())
{
  sub.actual().resize(combined_size_of(other, eN...));
}

} // namespace detail
} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
