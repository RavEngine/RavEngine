/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_mpl_if_t_h
#define	cml_common_mpl_if_t_h

#include <type_traits>

namespace cml {

/** Convenience alias to simplify conditionals.  If B is true, then if_t
 * takes the type of T.  Otherwise, if_t takes the type of F.
 */
template<bool B, class T, class F>
  using if_t = typename std::conditional<B,T,F>::type;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
