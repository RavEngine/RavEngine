/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_comparison_h
#define	cml_quaternion_comparison_h

#include <cml/quaternion/readable_quaternion.h>

namespace cml {

/** Returns true if @c left is lexicographically less than @c right. */
template<class Sub1, class Sub2> bool operator<(
  const readable_quaternion<Sub1>& left, const readable_quaternion<Sub2>& right);

/** Returns true if @c left is lexicographically less than or equal to @c
 * right.
 */
template<class Sub1, class Sub2> bool operator<=(
  const readable_quaternion<Sub1>& left, const readable_quaternion<Sub2>& right);

/** Returns true if @c left is lexicographically greater than @c right. */
template<class Sub1, class Sub2> bool operator>(
  const readable_quaternion<Sub1>& left, const readable_quaternion<Sub2>& right);

/** Returns true if @c left is lexicographically greater than or equal to
 * @c right.
 */
template<class Sub1, class Sub2> bool operator>=(
  const readable_quaternion<Sub1>& left, const readable_quaternion<Sub2>& right);

/** Returns true if the elements of @c left are all equal to the elements
 * of @c right.
 */
template<class Sub1, class Sub2> bool operator==(
  const readable_quaternion<Sub1>& left, const readable_quaternion<Sub2>& right);

/** Returns true if some element of @c left is not equal to the
 * corresponding element of @c right.
 */
template<class Sub1, class Sub2> bool operator!=(
  const readable_quaternion<Sub1>& left, const readable_quaternion<Sub2>& right);

} // namespace cml

#define __CML_QUATERNION_COMPARISON_TPP
#include <cml/quaternion/comparison.tpp>
#undef __CML_QUATERNION_COMPARISON_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
