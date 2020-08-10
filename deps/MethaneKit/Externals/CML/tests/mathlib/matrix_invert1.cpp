/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <iostream>

// Make sure the main header compiles cleanly:
#include <cml/mathlib/matrix/invert.h>

#include <cml/vector.h>
#include <cml/matrix.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("invert 2D, invert_2D_1")
{
  auto M = cml::matrix33d(
    1., 0., 3.,
    0., 1., 2.,
    0., 0., 1.
    );
  cml::matrix_invert_RT_only_2D(M);
  CATCH_CHECK(M.basis_element(2,0) == -3.);
  CATCH_CHECK(M.basis_element(2,1) == -2.);
}

CATCH_TEST_CASE("invert 2D, invert_nD_1")
{
  auto M = cml::matrix33d_r(
    1., 0., 0.,
    0., 1., 0.,
    3., 2., 1.
    );
  cml::matrix_invert_RT(M);
  CATCH_CHECK(M.basis_element(2,0) == -3.);
  CATCH_CHECK(M.basis_element(2,1) == -2.);
}




CATCH_TEST_CASE("invert 3D, invert_3D_1")
{
  auto M = cml::matrix44d(
    1., 0., 0., 3.,
    0., 1., 0., 2.,
    0., 0., 1., 1.,
    0., 0., 0., 1.
    );
  cml::matrix_invert_RT_only(M);
  CATCH_CHECK(M.basis_element(3,0) == -3.);
  CATCH_CHECK(M.basis_element(3,1) == -2.);
  CATCH_CHECK(M.basis_element(3,2) == -1.);
}

CATCH_TEST_CASE("invert 3D, invert_3D_2")
{
  auto M = cml::matrix44d_c(
    1., 0., 0., 3.,
    0., 1., 0., 2.,
    0., 0., 1., 1.,
    0., 0., 0., 1.
    );
  cml::matrix_invert_RT_only(M);
  CATCH_CHECK(M.basis_element(3,0) == -3.);
  CATCH_CHECK(M.basis_element(3,1) == -2.);
  CATCH_CHECK(M.basis_element(3,2) == -1.);
}

CATCH_TEST_CASE("invert 3D, invert_nD_1")
{
  auto M = cml::matrix44d_r(
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    3., 2., 1., 1.
    );
  cml::matrix_invert_RT(M);
  CATCH_CHECK(M.basis_element(3,0) == -3.);
  CATCH_CHECK(M.basis_element(3,1) == -2.);
  CATCH_CHECK(M.basis_element(3,2) == -1.);
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
