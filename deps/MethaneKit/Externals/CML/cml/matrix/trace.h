/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_trace_h
#define	cml_matrix_trace_h

#include <cml/common/traits.h>
#include <cml/matrix/fwd.h>

namespace cml {

/** Compute the trace of square matrix @c M.
 *
 * @throws non_square_matrix_error at run-time if the matrix is
 * dynamically-sized and not square.  Fixed-size matrices are checked at
 * compile-time.
 */
template<class Sub> auto
trace(const readable_matrix<Sub>& M) -> value_type_trait_of_t<Sub>;

} // namespace cml

#define __CML_MATRIX_TRACE_TPP
#include <cml/matrix/trace.tpp>
#undef __CML_MATRIX_TRACE_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
