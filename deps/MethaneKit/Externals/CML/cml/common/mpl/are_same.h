/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_mpl_are_same_h
#define	cml_common_mpl_are_same_h

#include <type_traits>

namespace cml {

/** Determine if a set of types, @c Froms, are the same as @c To via
 * std::is_same.
 */
template<class To, class... Froms> struct are_same;

/** Determine if @c From is the same as @c To. */
template<class To, class From>
  struct are_same<To,From> : std::is_same<From,To> {};

/** Recursively determine if @c From and @c Froms are the same as @c To. */
template<class To, class From, class... Froms>
struct are_same<To, From, Froms...> {
  static const bool value
    =  std::is_same<From,To>::value
    && are_same<To,Froms...>::value;
};

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
