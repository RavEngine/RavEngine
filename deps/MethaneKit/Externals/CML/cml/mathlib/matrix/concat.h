/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_matrix_concat_h
#define	cml_mathlib_matrix_concat_h

#include <cml/matrix/fwd.h>
#include <cml/matrix/promotion.h>

/** @defgroup mathlib_matrix_concat Ordered Matrix Concatenation Functions */

namespace cml {

/** @addtogroup mathlib_matrix_concat */
/*@{*/

/** Concatenate two square transformation matrices, @c m1 and @c m2, taking
 * basis orientation into account.
 *
 * - both col_basis, or col_basis with any_basis: return m1 * m2
 * - both row_basis, or row_basis with any_basis: return m2 * m1
 * - otherwise: compile-time error
 *
 * @throws non_square_matrix_error at run-time if @c m1 or @c m2 is
 * dynamically-sized and non-square.  The size is checked at compile-time
 * for fixed-size matrices.
 */
template<class Sub1, class Sub2> auto matrix_concat(
  const readable_matrix<Sub1>& m1, const readable_matrix<Sub2>& m2)
-> matrix_inner_product_promote_t<Sub1, Sub2>;

/*@}*/

} // namespace cml

#define __CML_MATHLIB_MATRIX_CONCAT_TPP
#include <cml/mathlib/matrix/concat.tpp>
#undef __CML_MATHLIB_MATRIX_CONCAT_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
