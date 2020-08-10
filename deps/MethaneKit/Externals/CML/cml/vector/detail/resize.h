/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_detail_resize_h
#define	cml_vector_detail_resize_h

#include <cml/vector/writable_vector.h>

namespace cml {
namespace detail {

/** No-op for non-resizable vector. */
template<class Sub> inline void resize(readable_vector<Sub>&, int) {}

/** Resize vectors that implement resize(). */
template<class Sub> inline auto resize(writable_vector<Sub>& sub, int size)
-> decltype(sub.actual().resize(0), void())
{
  sub.actual().resize(size);
}

} // namespace detail

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
