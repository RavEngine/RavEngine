/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/mathlib/matrix/scale.h>

#include <cml/vector.h>
#include <cml/matrix.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("scale 2D, set1")
{
  cml::matrix33d M;
  cml::matrix_scale_2D(M, 2., 2.);
  CATCH_CHECK(M(0,0) == 2.);
  CATCH_CHECK(M(1,1) == 2.);
}

CATCH_TEST_CASE("scale 2D, set2")
{
  cml::matrix33d M;
  cml::matrix_scale_2D(M, cml::vector2d(2., 2.));
  CATCH_CHECK(M(0,0) == 2.);
  CATCH_CHECK(M(1,1) == 2.);
}

CATCH_TEST_CASE("scale 2D, uniform1")
{
  cml::matrix33d M;
  cml::matrix_uniform_scale_2D(M, 2.);
  CATCH_CHECK(M(0,0) == 2.);
  CATCH_CHECK(M(1,1) == 2.);
}

CATCH_TEST_CASE("scale 2D, inverse1")
{
  cml::matrix33d M;
  cml::matrix_inverse_scale_2D(M, .5, .5);
  CATCH_CHECK(M(0,0) == 2.);
  CATCH_CHECK(M(1,1) == 2.);
}

CATCH_TEST_CASE("scale 2D, inverse2")
{
  cml::matrix33d M;
  cml::matrix_inverse_scale_2D(M, cml::vector2d(.5, .5));
  CATCH_CHECK(M(0,0) == 2.);
  CATCH_CHECK(M(1,1) == 2.);
}




CATCH_TEST_CASE("scale 3D, set1")
{
  cml::matrix44d M;
  cml::matrix_scale(M, 2., 2., 2.);
  CATCH_CHECK(M(0,0) == 2.);
  CATCH_CHECK(M(1,1) == 2.);
  CATCH_CHECK(M(2,2) == 2.);
}

CATCH_TEST_CASE("scale 3D, set2")
{
  cml::matrix44d M;
  cml::matrix_scale(M, cml::vector3d(2., 2., 2.));
  CATCH_CHECK(M(0,0) == 2.);
  CATCH_CHECK(M(1,1) == 2.);
  CATCH_CHECK(M(2,2) == 2.);
}

CATCH_TEST_CASE("scale 3D, uniform1")
{
  cml::matrix44d M;
  cml::matrix_uniform_scale(M, 2.);
  CATCH_CHECK(M(0,0) == 2.);
  CATCH_CHECK(M(1,1) == 2.);
  CATCH_CHECK(M(2,2) == 2.);
}

CATCH_TEST_CASE("scale 3D, inverse1")
{
  cml::matrix44d M;
  cml::matrix_inverse_scale(M, .5, .5, .5);
  CATCH_CHECK(M(0,0) == 2.);
  CATCH_CHECK(M(1,1) == 2.);
  CATCH_CHECK(M(2,2) == 2.);
}

CATCH_TEST_CASE("scale 3D, inverse2")
{
  cml::matrix44d M;
  cml::matrix_inverse_scale(M, cml::vector3d(.5, .5, .5));
  CATCH_CHECK(M(0,0) == 2.);
  CATCH_CHECK(M(1,1) == 2.);
  CATCH_CHECK(M(2,2) == 2.);
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
