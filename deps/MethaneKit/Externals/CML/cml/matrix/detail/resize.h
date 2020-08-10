/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_detail_resize_h
#define	cml_matrix_detail_resize_h

#include <cml/matrix/writable_matrix.h>

namespace cml {
namespace detail {

/** No-op for non-resizable matrices. */
template<class Sub> inline void resize(readable_matrix<Sub>&, int, int) {}

/** Resize matrices that implement resize(). */
template<class Sub> inline auto
resize(writable_matrix<Sub>& sub, int rows, int cols)
-> decltype(sub.actual().resize(0,0), void())
{
  sub.actual().resize(rows,cols);
}

} // namespace detail
} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
