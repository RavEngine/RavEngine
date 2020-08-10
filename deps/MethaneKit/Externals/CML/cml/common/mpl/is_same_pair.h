/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_mpl_is_same_pair_h
#define	cml_common_mpl_is_same_pair_h

#include <type_traits>

namespace cml {

/** Defines @c value to true if @c <T1,T2> is symmetrically identical to @c
 * <U1,U2>.
 */
template<class T1, class T2, class U1, class U2 = U1>
struct is_same_pair
{
  static const bool value
    =  (std::is_same<T1, U1>::value && std::is_same<T2, U2>::value)
    || (std::is_same<T1, U2>::value && std::is_same<T2, U1>::value);
};

/** Specialization of is_same_pair for matching @c U1 and @c U2. */
template<class T1, class T2, class U>
struct is_same_pair<T1, T2, U, U>
{
  static const bool value
    = std::is_same<T1, U>::value && std::is_same<T2, U>::value;
};

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
