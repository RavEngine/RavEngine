/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_matrix_basis_h
#define	cml_mathlib_matrix_basis_h

#include <cml/vector/fwd.h>
#include <cml/matrix/fwd.h>
#include <cml/mathlib/matrix/temporary.h>

/** @defgroup mathlib_matrix_basis Matrix Basis Functions */

namespace cml {

/** @addtogroup mathlib_matrix_basis */
/*@{*/

/** @defgroup mathlib_matrix_basis_2D 2D Matrix Basis Functions */
/*@{*/

/** Set the i'th basis vector of a 2D transform */
template<class Sub1, class Sub2> inline void
matrix_set_basis_vector_2D(
  writable_matrix<Sub1>& m, int i, const readable_vector<Sub2>& v);

/** Set the x basis vector of a 2D transform */
template<class Sub1, class Sub2> inline void
matrix_set_x_basis_vector_2D(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& x);

/** Set the y basis vector of a 2D transform */
template<class Sub1, class Sub2> inline void
matrix_set_y_basis_vector_2D(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& y);

/** Set the basis vectors of 2D transform @c m. */
template<class Sub1, class SubX, class SubY> inline void
matrix_set_basis_vectors_2D(writable_matrix<Sub1>& m,
  const readable_vector<SubX>& x, const readable_vector<SubY>& y);


/** Set the i'th transposed basis vector of a 2D transform */
template<class Sub1, class Sub2> inline void
matrix_set_transposed_basis_vector_2D(
  writable_matrix<Sub1>& m, int i, const readable_vector<Sub2>& v);

/** Set the transposed x basis vector of a 2D transform */
template<class Sub1, class Sub2> inline void
matrix_set_transposed_x_basis_vector_2D(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& x);

/** Set the transposed y basis vector of a 2D transform */
template<class Sub1, class Sub2> inline void
matrix_set_transposed_y_basis_vector_2D(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& y);

/** Set the transposed basis vectors of 2D transform @c m. */
template<class Sub1, class SubX, class SubY> inline void
matrix_set_transposed_basis_vectors_2D(writable_matrix<Sub1>& m,
  const readable_vector<SubX>& x, const readable_vector<SubY>& y);


/** Get the i'th basis vector of a 2D transform as a temporary vector. */
template<class Sub> inline auto
matrix_get_basis_vector_2D(const readable_matrix<Sub>& m, int i)
-> n_basis_vector_of_t<Sub,2>;

/** Get the x basis vector of a 2D transform as a temporary vector. */
template<class Sub> inline auto
matrix_get_x_basis_vector_2D(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,2>;

/** Get the y basis vector of a 2D transform as a temporary vector. */
template<class Sub> inline auto
matrix_get_y_basis_vector_2D(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,2>;

/** Get the basis vectors of 2D transform @c m. */
template<class Sub, class SubX, class SubY> inline void
matrix_get_basis_vectors_2D(const readable_matrix<Sub>& m,
  writable_vector<SubX>& x, writable_vector<SubY>& y);


/** Get the i'th transposed basis vector of a 2D transform as a temporary
 * vector.
 */
template<class Sub> inline auto
matrix_get_transposed_basis_vector_2D(const readable_matrix<Sub>& m, int i)
-> n_basis_vector_of_t<Sub,2>;

/** Get the transposed x basis vector of a 2D transform as a temporary
 * vector.
 */
template<class Sub> inline auto
matrix_get_transposed_x_basis_vector_2D(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,2>;

/** Get the transposed y basis vector of a 2D transform as a temporary
 * vector.
 */
template<class Sub> inline auto
matrix_get_transposed_y_basis_vector_2D(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,2>;

/** Get the transposed basis vectors of 2D transform @c m. */
template<class Sub, class SubX, class SubY> inline void
matrix_get_transposed_basis_vectors_2D(const readable_matrix<Sub>& m,
  writable_vector<SubX>& v1, writable_vector<SubY>& v2);

/*@}*/ // mathlib_matrix_basis_2D


/** @defgroup mathlib_matrix_basis_3D 3D Matrix Basis Functions */
/*@{*/

/** Set the i'th basis vector of a 3D transform */
template<class Sub1, class Sub2> inline void
matrix_set_basis_vector(
  writable_matrix<Sub1>& m, int i, const readable_vector<Sub2>& v);

/** Set the x basis vector of a 3D transform */
template<class Sub1, class Sub2> inline void
matrix_set_x_basis_vector(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& x);

/** Set the y basis vector of a 3D transform */
template<class Sub1, class Sub2> inline void
matrix_set_y_basis_vector(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& y);

/** Set the z basis vector of a 3D transform */
template<class Sub1, class Sub2> inline void
matrix_set_z_basis_vector(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& z);

/** Set the basis vectors of 3D transform @c m. */
template<class Sub1, class SubX, class SubY, class SubZ> inline void
matrix_set_basis_vectors(
  writable_matrix<Sub1>& m, const readable_vector<SubX>& x,
  const readable_vector<SubY>& y, const readable_vector<SubZ>& z);


/** Set the i'th transposed basis vector of a 3D transform */
template<class Sub1, class Sub2> inline void
matrix_set_transposed_basis_vector(
  writable_matrix<Sub1>& m, int i, const readable_vector<Sub2>& v);

/** Set the transposed x basis vector of a 3D transform */
template<class Sub1, class Sub2> inline void
matrix_set_transposed_x_basis_vector(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& x);

/** Set the transposed y basis vector of a 3D transform */
template<class Sub1, class Sub2> inline void
matrix_set_transposed_y_basis_vector(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& y);

/** Set the transposed z basis vector of a 3D transform */
template<class Sub1, class Sub2> inline void
matrix_set_transposed_z_basis_vector(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& z);

/** Set the transposed basis vectors of 3D transform @c m. */
template<class Sub1, class SubX, class SubY, class SubZ> inline void
matrix_set_transposed_basis_vectors(
  writable_matrix<Sub1>& m, const readable_vector<SubX>& x,
  const readable_vector<SubY>& y, const readable_vector<SubZ>& z);


/** Get the i'th basis vector of a 3D transform as a temporary vector. */
template<class Sub> inline auto
matrix_get_basis_vector(const readable_matrix<Sub>& m, int i)
-> n_basis_vector_of_t<Sub,3>;

/** Get the x basis vector of a 3D transform as a temporary vector. */
template<class Sub> inline auto
matrix_get_x_basis_vector(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,3>;

/** Get the y basis vector of a 3D transform as a temporary vector. */
template<class Sub> inline auto
matrix_get_y_basis_vector(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,3>;

/** Get the z basis vector of a 3D transform as a temporary vector. */
template<class Sub> inline auto
matrix_get_z_basis_vector(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,3>;

/** Get the basis vectors of a 3D transform */
template<class Sub, class SubX, class SubY, class SubZ> inline void
matrix_get_basis_vectors(
  const readable_matrix<Sub>& m, writable_vector<SubX>& v1,
  writable_vector<SubY>& v2, writable_vector<SubZ>& v3);


/** Get the i'th transposed basis vector of a 3D transform as a temporary
 * vector.
 */
template<class Sub> inline auto
matrix_get_transposed_basis_vector(const readable_matrix<Sub>& m, int i)
-> n_basis_vector_of_t<Sub,3>;

/** Get the transposed x basis vector of a 3D transform as a temporary
 * vector.
 */
template<class Sub> inline auto
matrix_get_transposed_x_basis_vector(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,3>;

/** Get the transposed y basis vector of a 3D transform as a temporary
 * vector.
 */
template<class Sub> inline auto
matrix_get_transposed_y_basis_vector(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,3>;

/** Get the transposed z basis vector of a 3D transform as a temporary
 * vector.
 */
template<class Sub> inline auto
matrix_get_transposed_z_basis_vector(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,3>;

/** Get the transposed basis vectors of 3D transform @c m. */
template<class Sub, class SubX, class SubY, class SubZ> inline void
matrix_get_transposed_basis_vectors(
  const readable_matrix<Sub>& m, writable_vector<SubX>& x,
  writable_vector<SubY>& y, writable_vector<SubZ>& z);

/*@}*/ // mathlib_matrix_basis_3D


/** @defgroup mathlib_matrix_basis_nD nD Matrix Basis Functions */
/*@{*/

template<class Sub> auto
matrix_get_basis_vector_nD(const readable_matrix<Sub>& m, int i)
-> basis_vector_of_t<Sub>;

/*@}*/ // mathlib_matrix_basis_nD

/*@}*/ // mathlib_matrix_basis

} // namespace cml

#define __CML_MATHLIB_MATRIX_BASIS_TPP
#include <cml/mathlib/matrix/basis.tpp>
#undef __CML_MATHLIB_MATRIX_BASIS_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
