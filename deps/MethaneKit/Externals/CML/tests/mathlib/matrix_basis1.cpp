/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/mathlib/matrix/basis.h>

#include <cml/vector.h>
#include <cml/matrix.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("basis 2D, basis1")
{
  cml::matrix22d M;	// column basis

  cml::matrix_set_x_basis_vector_2D(M, cml::vector2d(1., 2.));
  cml::matrix_set_y_basis_vector_2D(M, cml::vector2d(3., 4.));

  auto b1 = cml::matrix_get_x_basis_vector_2D(M);
  auto b2 = cml::matrix_get_y_basis_vector_2D(M);

  CATCH_CHECK(b1[0] == 1.);
  CATCH_CHECK(b1[1] == 2.);
  CATCH_CHECK(b2[0] == 3.);
  CATCH_CHECK(b2[1] == 4.);
}

CATCH_TEST_CASE("basis 2D, basis2")
{
  cml::matrix22d M;	// column basis

  cml::matrix_set_transposed_x_basis_vector_2D(M, cml::vector2d(1., 2.));
  cml::matrix_set_transposed_y_basis_vector_2D(M, cml::vector2d(3., 4.));

  auto b1 = cml::matrix_get_transposed_x_basis_vector_2D(M);
  auto b2 = cml::matrix_get_transposed_y_basis_vector_2D(M);

  CATCH_CHECK(b1[0] == 1.);
  CATCH_CHECK(b1[1] == 2.);
  CATCH_CHECK(b2[0] == 3.);
  CATCH_CHECK(b2[1] == 4.);
}

CATCH_TEST_CASE("basis 2D, basis3")
{
  cml::matrix22d M;	// column basis

  cml::matrix_set_basis_vectors_2D(
    M, cml::vector2d(1., 2.), cml::vector2d(3., 4.));

  cml::vectord b1, b2;
  cml::matrix_get_basis_vectors_2D(M, b1, b2);

  CATCH_CHECK(b1[0] == 1.);
  CATCH_CHECK(b1[1] == 2.);
  CATCH_CHECK(b2[0] == 3.);
  CATCH_CHECK(b2[1] == 4.);
}

CATCH_TEST_CASE("basis 2D, basis4")
{
  cml::matrix22d M;	// column basis

  cml::matrix_set_transposed_basis_vectors_2D(
    M, cml::vector2d(1., 2.), cml::vector2d(3., 4.));

  cml::vectord b1, b2;
  cml::matrix_get_transposed_basis_vectors_2D(M, b1, b2);

  CATCH_CHECK(b1[0] == 1.);
  CATCH_CHECK(b1[1] == 2.);
  CATCH_CHECK(b2[0] == 3.);
  CATCH_CHECK(b2[1] == 4.);
}




CATCH_TEST_CASE("basis 3D, basis1")
{
  cml::matrix33d M;	// column basis

  cml::matrix_set_x_basis_vector(M, cml::vector3d(1., 2., 3.));
  cml::matrix_set_y_basis_vector(M, cml::vector3d(4., 5., 6.));
  cml::matrix_set_z_basis_vector(M, cml::vector3d(7., 8., 9.));

  auto b1 = cml::matrix_get_x_basis_vector(M);
  auto b2 = cml::matrix_get_y_basis_vector(M);
  auto b3 = cml::matrix_get_z_basis_vector(M);

  CATCH_CHECK(b1[0] == 1.);
  CATCH_CHECK(b1[1] == 2.);
  CATCH_CHECK(b1[2] == 3.);

  CATCH_CHECK(b2[0] == 4.);
  CATCH_CHECK(b2[1] == 5.);
  CATCH_CHECK(b2[2] == 6.);

  CATCH_CHECK(b3[0] == 7.);
  CATCH_CHECK(b3[1] == 8.);
  CATCH_CHECK(b3[2] == 9.);
}

CATCH_TEST_CASE("basis 3D, basis2")
{
  cml::matrix33d M;	// column basis

  cml::matrix_set_transposed_x_basis_vector(M, cml::vector3d(1., 2., 3.));
  cml::matrix_set_transposed_y_basis_vector(M, cml::vector3d(4., 5., 6.));
  cml::matrix_set_transposed_z_basis_vector(M, cml::vector3d(7., 8., 9.));

  auto b1 = cml::matrix_get_transposed_x_basis_vector(M);
  auto b2 = cml::matrix_get_transposed_y_basis_vector(M);
  auto b3 = cml::matrix_get_transposed_z_basis_vector(M);

  CATCH_CHECK(b1[0] == 1.);
  CATCH_CHECK(b1[1] == 2.);
  CATCH_CHECK(b1[2] == 3.);

  CATCH_CHECK(b2[0] == 4.);
  CATCH_CHECK(b2[1] == 5.);
  CATCH_CHECK(b2[2] == 6.);

  CATCH_CHECK(b3[0] == 7.);
  CATCH_CHECK(b3[1] == 8.);
  CATCH_CHECK(b3[2] == 9.);
}

CATCH_TEST_CASE("basis 3D, basis3")
{
  cml::matrix33d M;	// column basis

  cml::matrix_set_basis_vectors(M, cml::vector3d(1., 2., 3.),
    cml::vector3d(4., 5., 6.), cml::vector3d(7., 8., 9.));

  cml::vectord b1, b2, b3;
  cml::matrix_get_basis_vectors(M, b1, b2, b3);

  CATCH_CHECK(b1[0] == 1.);
  CATCH_CHECK(b1[1] == 2.);
  CATCH_CHECK(b1[2] == 3.);

  CATCH_CHECK(b2[0] == 4.);
  CATCH_CHECK(b2[1] == 5.);
  CATCH_CHECK(b2[2] == 6.);

  CATCH_CHECK(b3[0] == 7.);
  CATCH_CHECK(b3[1] == 8.);
  CATCH_CHECK(b3[2] == 9.);
}

CATCH_TEST_CASE("basis 3D, basis4")
{
  cml::matrix33d M;	// column basis

  cml::matrix_set_transposed_basis_vectors(M, cml::vector3d(1., 2., 3.),
    cml::vector3d(4., 5., 6.), cml::vector3d(7., 8., 9.));

  cml::vectord b1, b2, b3;
  cml::matrix_get_transposed_basis_vectors(M, b1, b2, b3);

  CATCH_CHECK(b1[0] == 1.);
  CATCH_CHECK(b1[1] == 2.);
  CATCH_CHECK(b1[2] == 3.);

  CATCH_CHECK(b2[0] == 4.);
  CATCH_CHECK(b2[1] == 5.);
  CATCH_CHECK(b2[2] == 6.);

  CATCH_CHECK(b3[0] == 7.);
  CATCH_CHECK(b3[1] == 8.);
  CATCH_CHECK(b3[2] == 9.);
}




CATCH_TEST_CASE("basis nD, basis1")
{
  auto M = cml::matrix34d_c(
    1., 0., 0., 3.,
    0., 1., 0., 2.,
    0., 0., 1., 1.
    );
  auto T = cml::matrix_get_basis_vector_nD(M, 3);
  CATCH_CHECK(T[0] == 3.);
  CATCH_CHECK(T[1] == 2.);
  CATCH_CHECK(T[2] == 1.);
}

CATCH_TEST_CASE("basis nD, basis2")
{
  auto M = cml::matrix43d_r(
    1., 0., 0.,
    0., 1., 0.,
    0., 0., 1.,
    3., 2., 1.
    );
  auto T = cml::matrix_get_basis_vector_nD(M, 3);
  CATCH_CHECK(T[0] == 3.);
  CATCH_CHECK(T[1] == 2.);
  CATCH_CHECK(T[2] == 1.);
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
