/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_mpl_enable_if_t_h
#define	cml_common_mpl_enable_if_t_h

#include <type_traits>

namespace cml {

/** Helper for enable_if.
 *
 * @todo use C++14 enable_if_t if available.
 */
template<bool B, class T = void>
  using enable_if_t = typename std::enable_if<B,T>::type;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
