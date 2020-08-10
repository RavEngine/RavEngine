/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
//#include <cml/matrix/functions.h>

#include <cml/vector/fixed.h>
#include <cml/matrix/fixed.h>
#include <cml/types.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("fixed, zero1")
{
  cml::matrix22d M;
  M.zero();
  CATCH_CHECK(M(0,0) == 0.);
  CATCH_CHECK(M(0,1) == 0.);
  CATCH_CHECK(M(1,0) == 0.);
  CATCH_CHECK(M(1,1) == 0.);
}

CATCH_TEST_CASE("fixed, identity1")
{
  cml::matrix22d M;
  M.identity();
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(0,1) == 0.);
  CATCH_CHECK(M(1,0) == 0.);
  CATCH_CHECK(M(1,1) == 1.);
}

CATCH_TEST_CASE("fixed, random1")
{
  cml::matrix22d M;
  M.random(0.,1.);
  for(const auto& v : M) {
    CATCH_CHECK(v >= 0.);
    CATCH_CHECK(v < 1.);
  }
}

CATCH_TEST_CASE("fixed, set_basis_element1")
{
  cml::matrix34d_c M;
  M.set_basis_element(0, 0, 1.);
  M.set_basis_element(0, 1, 2.);
  M.set_basis_element(0, 2, 3.);

  CATCH_CHECK(M.basis_element(0,0) == 1.);
  CATCH_CHECK(M.basis_element(0,1) == 2.);
  CATCH_CHECK(M.basis_element(0,2) == 3.);

  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(1,0) == 2.);
  CATCH_CHECK(M(2,0) == 3.);
}

CATCH_TEST_CASE("fixed, set_basis_element2")
{
  cml::matrix43d_r M;
  M.set_basis_element(0, 0, 1.);
  M.set_basis_element(0, 1, 2.);
  M.set_basis_element(0, 2, 3.);

  CATCH_CHECK(M.basis_element(0,0) == 1.);
  CATCH_CHECK(M.basis_element(0,1) == 2.);
  CATCH_CHECK(M.basis_element(0,2) == 3.);

  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(0,1) == 2.);
  CATCH_CHECK(M(0,2) == 3.);
}

CATCH_TEST_CASE("fixed, set_row1")
{
  using cml::vector3d;
  cml::matrix33d M;
  M.set_row(0, vector3d(1., 2., 3.));
  M.set_row(1, vector3d(4., 5., 6.));
  M.set_row(2, vector3d(7., 8., 9.));
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(0,1) == 2.);
  CATCH_CHECK(M(1,0) == 4.);
  CATCH_CHECK(M(0,2) == 3.);
  CATCH_CHECK(M(2,0) == 7.);
}

CATCH_TEST_CASE("fixed, set_col1")
{
  using cml::vector3d;
  cml::matrix33d M;
  M.set_col(0, vector3d(1., 4., 7.));
  M.set_col(1, vector3d(2., 5., 8.));
  M.set_col(2, vector3d(3., 6., 9.));
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(0,1) == 2.);
  CATCH_CHECK(M(1,0) == 4.);
  CATCH_CHECK(M(0,2) == 3.);
  CATCH_CHECK(M(2,0) == 7.);
}

CATCH_TEST_CASE("fixed, trace1")
{
  auto M = cml::matrix44d(
    242.,  20., -33., -1.,
     12., -48.,  45., -9.,
     46., -32., -27., 13.,
    -44.,  43.,   6., -5.
    );
  double expected = 242. - 48. - 27. - 5.;
  CATCH_CHECK(M.trace() == Approx(expected).epsilon(1e-12));
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
