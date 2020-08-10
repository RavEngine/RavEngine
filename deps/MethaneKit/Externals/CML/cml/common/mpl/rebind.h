/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_mpl_rebind_h
#define	cml_common_mpl_rebind_h

#include <memory>
#include <cml/common/compiler.h>

namespace cml {

/** Helper to rebind @c T via template rebind<Args...>.
 *
 * @note Only typed parameters are supported.
 */
template<class T, class... Args> struct rebind {
  using other = typename T::template rebind<Args...>::other;
};

/** Convenience alias for rebind. */
template<class T, class... Args>
  using rebind_t = typename rebind<T,Args...>::other;


#ifdef CML_HAS_MSVC_WONKY_PARAMETER_PACK
/** Helper to rebind allocator @c T.
 *
 * @note Only typed parameters are supported.
 */
template<class T, class Arg0> struct rebind_alloc {
  using traits = std::allocator_traits<T>;
  using other = typename traits::template rebind_alloc<Arg0>;
};

/** Convenience alias for rebind_alloc. */
template<class T, class... Args>
  using rebind_alloc_t = typename rebind_alloc<T,Args...>::other;
#else
/** Helper to rebind allocator @c T.
 *
 * @note Only typed parameters are supported.
 */
template<class T, class... Args> struct rebind_alloc {
  using traits = std::allocator_traits<T>;
  using other = typename traits::template rebind_alloc<Args...>;
};

/** Convenience alias for rebind_alloc. */
template<class T, class... Args>
  using rebind_alloc_t = typename rebind_alloc<T,Args...>::other;
#endif

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
