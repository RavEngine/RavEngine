/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_vector_orthonormal_h
#define	cml_mathlib_vector_orthonormal_h

#include <cml/vector/fwd.h>
#include <cml/mathlib/axis_order.h>

/** @defgroup mathlib_vector_ortho Vector Orthonormalization Functions */

namespace cml {

/** @addtogroup mathlib_vector_ortho */
/*@{*/

/** @defgroup mathlib_vector_ortho_basis_2D 2D Orthonormal Basis Construction */
/*@{*/

/** Build a 2D orthonormal basis. */
template<class Sub, class XSub, class YSub> void
orthonormal_basis_2D(
  const readable_vector<Sub>& align,
  writable_vector<XSub>& x, writable_vector<YSub>& y,
  bool normalize_align = true, axis_order2D order = axis_order_xy);

/*@}*/

/** @defgroup mathlib_vector_ortho_basis_3D 3D Orthonormal Basis Construction */
/*@{*/

/** This version of orthonormal_basis() ultimately does the work for all
 * orthonormal_basis_*() functions. Given input vectors 'align' and
 * 'reference', and an order 'axis_order_\<i\>\<j\>\<k\>', it constructs an
 * orthonormal basis such that the i'th basis vector is aligned with
 * (parallel to and pointing in the same direction as) 'align', and the
 * j'th basis vector is maximally aligned with 'reference'. The k'th basis
 * vector is chosen such that the basis has a determinant of +1.
 *
 * @note The algorithm fails when 'align' is nearly parallel to
 * 'reference'; this should be checked for and handled externally if it's a
 * case that may occur.
 */
template<class Sub1, class Sub2, class XSub, class YSub, class ZSub> void
orthonormal_basis(
  const readable_vector<Sub1>& align, const readable_vector<Sub2>& reference,
  writable_vector<XSub>& x, writable_vector<YSub>& y, writable_vector<ZSub>& z,
  bool normalize_align = true, axis_order order = axis_order_zyx);

/*@}*/

/*@}*/

} // namespace cml

#define __CML_MATHLIB_VECTOR_ORTHONORMAL_TPP
#include <cml/mathlib/vector/orthonormal.tpp>
#undef __CML_MATHLIB_VECTOR_ORTHONORMAL_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
