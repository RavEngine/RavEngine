/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_mpl_enable_if_pointer_h
#define	cml_common_mpl_enable_if_pointer_h

#include <type_traits>

namespace cml {

/** Alias for std::enable_if<T> using std::is_pointer<T> to determine the
 * boolean value.
 */
template<class Pointer> using enable_if_pointer
  = typename std::enable_if<
  std::is_pointer<Pointer>::value && !std::is_array<Pointer>::value>;

/** Alias for std::enable_if<T>::type using std::is_pointer<T> to determine
 * the boolean value.
 */
template<class Pointer> using enable_if_pointer_t
  = typename enable_if_pointer<Pointer>::type;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
