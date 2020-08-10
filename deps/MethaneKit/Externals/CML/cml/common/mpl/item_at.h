/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_mpl_item_at_h
#define	cml_common_mpl_item_at_h

#include <tuple>

namespace cml {

/** Return item @c N of argument pack @c Args. */
template<int N, class... Args> inline auto item_at(Args&&... args)
-> decltype(std::get<N>(std::forward_as_tuple(std::forward<Args>(args)...)))
{
  return std::get<N>(std::forward_as_tuple(std::forward<Args>(args)...));
}

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
