/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_mpl_enable_if_same_h
#define	cml_common_mpl_enable_if_same_h

#include <cml/common/mpl/are_same.h>

namespace cml {

/** Wrapper for enable_if to detect if a set of types, @c Froms, are the
 * same type as @c To.
 */
template<class To, class... Froms> struct enable_if_same
  : std::enable_if<are_same<To, Froms...>::value> {};

/** Convenience alias for enable_if_same. */
template<class To, class... Froms> using enable_if_same_t
  = typename enable_if_same<To, Froms...>::type;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
