/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_quaternion_basis_h
#define	cml_mathlib_quaternion_basis_h

#include <cml/vector/temporary.h>
#include <cml/quaternion/readable_quaternion.h>

/** @defgroup mathlib_quaternion_basis Quaternion Basis Vector Functions */

namespace cml {

/** @addtogroup mathlib_quaternion_basis */
/*@{*/

/** Get basis vector @c i of quaternion rotation @c q.
 *
 * @throws std::invalid_argument if @c i < 0 or @c i > 2.
 */
template<class Sub> auto
quaternion_get_basis_vector(const readable_quaternion<Sub>& q, int i)
-> temporary_of_t<decltype(q.imaginary())>;

/** Get the x-basis vector of @c q. */
template<class Sub> auto
quaternion_get_x_basis_vector(const readable_quaternion<Sub>& q)
-> temporary_of_t<decltype(q.imaginary())>;

/** Get the y-basis vector of @c q. */
template<class Sub> auto
quaternion_get_y_basis_vector(const readable_quaternion<Sub>& q)
-> temporary_of_t<decltype(q.imaginary())>;

/** Get the z-basis vector of @c q. */
template<class Sub> auto
quaternion_get_z_basis_vector(const readable_quaternion<Sub>& q)
-> temporary_of_t<decltype(q.imaginary())>;

/** Return the basis vectors of @c q as three vectors, @c x, @c y, and @c
 * z.
 *
 * @throws vector_size_error at run-time if any of @c x, @c y, or @c z is
 * dynamically-sized, and is not 3D.  Fixed-size vectors are checked at
 * compile-time.
 */
template<class Sub, class XBasis, class YBasis, class ZBasis,
  enable_if_vector_t<XBasis>* = nullptr,
  enable_if_vector_t<YBasis>* = nullptr,
  enable_if_vector_t<ZBasis>* = nullptr>
void quaternion_get_basis_vectors(
  const readable_quaternion<Sub>& q, XBasis& x, YBasis& y, ZBasis& z);

/*@}*/

} // namespace cml

#define __CML_MATHLIB_QUATERNION_BASIS_TPP
#include <cml/mathlib/quaternion/basis.tpp>
#undef __CML_MATHLIB_QUATERNION_BASIS_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
