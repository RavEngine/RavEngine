/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <cml/vector/dot.h>
#include <cml/mathlib/matrix/basis.h>
#include <cml/mathlib/matrix/translation.h>

namespace cml {

template<class Sub> void
matrix_invert_RT_only_2D(writable_matrix<Sub>& m)
{
  cml::check_affine_2D(m);

  /* Transpose the basis vectors: */
  auto x = matrix_get_x_basis_vector_2D(m);
  auto y = matrix_get_y_basis_vector_2D(m);
  matrix_set_transposed_basis_vectors_2D(m, x,y);

  /* Transform the translation: */
  auto p = matrix_get_translation_2D(m);
  matrix_set_translation_2D(m, - dot(p,x), - dot(p,y));
}

template<class Sub> void
matrix_invert_RT_only(writable_matrix<Sub>& m)
{
  cml::check_affine_3D(m);

  /* Transpose the basis vectors: */
  auto x = matrix_get_x_basis_vector(m);
  auto y = matrix_get_y_basis_vector(m);
  auto z = matrix_get_z_basis_vector(m);
  matrix_set_transposed_basis_vectors(m, x,y,z);

  /* Transform the translation: */
  auto p = matrix_get_translation(m);
  matrix_set_translation(m, - dot(p,x), - dot(p,y), - dot(p,z));
}

template<class Sub> void
matrix_invert_RT(writable_matrix<Sub>& m)
{
  cml::check_affine(m);

  int R = m.rows(), C = m.cols();
  int M;
  /**/ if(R > C) M = C;		// Rectangular, row basis, e.g. 4x3
  else if(R < C) M = R;		// Rectangular, col basis, e.g. 3x4
  else M = R-1;			// Square, either, e.g. 4x4.

  /* Transpose the MxM rotation part of m in-place: */
  for(int i = 0; i < M; ++ i) {
    for(int j = i+1; j < M; ++ j) {
      auto e_ij = m.basis_element(i,j);
      auto e_ji = m.basis_element(j,i);
      m.set_basis_element(i,j, e_ji);
      m.set_basis_element(j,i, e_ij);
    }
  }

  /* Negate the Mx1 translation (basis vector M) and multiply by the
   * transposed rotation:
   */
  auto T = matrix_get_basis_vector_nD(m, M);
  for(int i = 0; i < M; ++ i) {
    auto e = m.basis_element(0, i) * T[0];
    for(int j = 1; j < M; ++ j) e += m.basis_element(j,i) * T[j];
    m.set_basis_element(M, i, - e);
  }
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
