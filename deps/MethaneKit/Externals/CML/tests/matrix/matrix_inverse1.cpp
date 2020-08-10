/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/matrix/inverse.h>

#include <cml/matrix/fixed.h>
#include <cml/matrix/dynamic.h>
#include <cml/matrix/external.h>
#include <cml/matrix/types.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("fixed, inverse_assign_2x2")
{
  cml::matrix22d M(
    1., 2.,
    3., 4.
    );
  M.inverse();
  CATCH_CHECK(M(0,0) == Approx(-2.0).epsilon(1e-12));
  CATCH_CHECK(M(0,1) == Approx( 1.0).epsilon(1e-12));
  CATCH_CHECK(M(1,0) == Approx( 1.5).epsilon(1e-12));
  CATCH_CHECK(M(1,1) == Approx(-0.5).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, inverse_assign_3x3")
{
  cml::matrix33d M(
    1.,  2.,  3.,
    1.,  4.,  9.,
    1., 16., 25.
    );
  M.inverse();

  auto expected = cml::matrix33d(
    22.,   1., -3.,
     8., -11.,  3.,
    -6.,   7., -1.
    );
  expected *= 1./20.;

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(M(i,j) == Approx(expected(i,j)).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, inverse_assign_4x4")
{
  cml::matrix44d M(
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
    );
  M.inverse();

  auto expected = cml::matrix44d(
    242.,  20., -33., -1.,
     12., -48.,  45., -9.,
     46., -32., -27., 13.,
    -44.,  43.,   6., -5.
    );
  expected *= 1./228.;

  for(int i = 0; i < 4; ++ i)
    for(int j = 0; j < 4; ++ j)
      CATCH_CHECK(M(i,j) == Approx(expected(i,j)).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, inverse_2x2")
{
  auto M = cml::inverse(
    cml::matrix22d(
      1., 2.,
      3., 4.
      )
    );
  CATCH_CHECK(M(0,0) == Approx(-2.0).epsilon(1e-12));
  CATCH_CHECK(M(0,1) == Approx( 1.0).epsilon(1e-12));
  CATCH_CHECK(M(1,0) == Approx( 1.5).epsilon(1e-12));
  CATCH_CHECK(M(1,1) == Approx(-0.5).epsilon(1e-12));
}




CATCH_TEST_CASE("fixed external, inverse_assign_2x2")
{
  double avM[] = {
    1., 2.,
    3., 4.
  };
  cml::external22d M(avM);
  M.inverse();
  CATCH_CHECK(M(0,0) == Approx(-2.0).epsilon(1e-12));
  CATCH_CHECK(M(0,1) == Approx( 1.0).epsilon(1e-12));
  CATCH_CHECK(M(1,0) == Approx( 1.5).epsilon(1e-12));
  CATCH_CHECK(M(1,1) == Approx(-0.5).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed external, inverse_assign_3x3")
{
  double avM[] = {
    1.,  2.,  3.,
    1.,  4.,  9.,
    1., 16., 25.
  };
  cml::external33d M(avM);
  M.inverse();

  auto expected = cml::matrix33d(
    22.,   1., -3.,
     8., -11.,  3.,
    -6.,   7., -1.
    );
  expected *= 1./20.;

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(M(i,j) == Approx(expected(i,j)).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed external, inverse_assign_4x4")
{
  double avM[] = {
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
  };
  cml::external44d M(avM);
  M.inverse();

  auto expected = cml::matrix44d(
    242.,  20., -33., -1.,
     12., -48.,  45., -9.,
     46., -32., -27., 13.,
    -44.,  43.,   6., -5.
    );
  expected *= 1./228.;

  for(int i = 0; i < 4; ++ i)
    for(int j = 0; j < 4; ++ j)
      CATCH_CHECK(M(i,j) == Approx(expected(i,j)).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed external, inverse_2x2")
{
  double avM[] = {
    1., 2.,
    3., 4.
  };
  auto M = cml::inverse(cml::external22d(avM));
  CATCH_CHECK(M(0,0) == Approx(-2.0).epsilon(1e-12));
  CATCH_CHECK(M(0,1) == Approx( 1.0).epsilon(1e-12));
  CATCH_CHECK(M(1,0) == Approx( 1.5).epsilon(1e-12));
  CATCH_CHECK(M(1,1) == Approx(-0.5).epsilon(1e-12));
}




CATCH_TEST_CASE("dynamic external, inverse_assign_2x2")
{
  double avM[] = {
    1., 2.,
    3., 4.
  };
  cml::externalmnd M(2,2, avM);
  M.inverse();
  CATCH_CHECK(M(0,0) == Approx(-2.0).epsilon(1e-12));
  CATCH_CHECK(M(0,1) == Approx( 1.0).epsilon(1e-12));
  CATCH_CHECK(M(1,0) == Approx( 1.5).epsilon(1e-12));
  CATCH_CHECK(M(1,1) == Approx(-0.5).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic external, inverse_assign_3x3")
{
  double avM[] = {
    1.,  2.,  3.,
    1.,  4.,  9.,
    1., 16., 25.
  };
  cml::externalmnd M(3,3, avM);
  M.inverse();

  auto expected = cml::matrix33d(
    22.,   1., -3.,
     8., -11.,  3.,
    -6.,   7., -1.
    );
  expected *= 1./20.;

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(M(i,j) == Approx(expected(i,j)).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic external, inverse_assign_4x4")
{
  double avM[] = {
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
  };
  cml::externalmnd M(4,4, avM);
  M.inverse();

  auto expected = cml::matrix44d(
    242.,  20., -33., -1.,
     12., -48.,  45., -9.,
     46., -32., -27., 13.,
    -44.,  43.,   6., -5.
    );
  expected *= 1./228.;

  for(int i = 0; i < 4; ++ i)
    for(int j = 0; j < 4; ++ j)
      CATCH_CHECK(M(i,j) == Approx(expected(i,j)).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic external, inverse_2x2")
{
  double avM[] = {
    1., 2.,
    3., 4.
  };
  auto M = cml::inverse(cml::externalmnd(2,2, avM));
  CATCH_CHECK(M(0,0) == Approx(-2.0).epsilon(1e-12));
  CATCH_CHECK(M(0,1) == Approx( 1.0).epsilon(1e-12));
  CATCH_CHECK(M(1,0) == Approx( 1.5).epsilon(1e-12));
  CATCH_CHECK(M(1,1) == Approx(-0.5).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic external, size_check1")
{
  double avM[3*4];
  cml::externalmnd M(3,4, avM);
  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_REQUIRE_THROWS_AS(M.inverse(), cml::non_square_matrix_error);
}




CATCH_TEST_CASE("dynamic, inverse_assign_2x2")
{
  cml::matrixd M(
    2,2,
    1., 2.,
    3., 4.
    );
  M.inverse();
  CATCH_CHECK(M(0,0) == Approx(-2.0).epsilon(1e-12));
  CATCH_CHECK(M(0,1) == Approx( 1.0).epsilon(1e-12));
  CATCH_CHECK(M(1,0) == Approx( 1.5).epsilon(1e-12));
  CATCH_CHECK(M(1,1) == Approx(-0.5).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic, inverse_assign_3x3")
{
  cml::matrixd M(
    3,3,
    1.,  2.,  3.,
    1.,  4.,  9.,
    1., 16., 25.
    );
  M.inverse();

  auto expected = cml::matrix33d(
    22.,   1., -3.,
     8., -11.,  3.,
    -6.,   7., -1.
    );
  expected *= 1./20.;

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(M(i,j) == Approx(expected(i,j)).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic, inverse_assign_4x4")
{
  cml::matrixd M(
    4,4,
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
    );
  M.inverse();

  auto expected = cml::matrix44d(
    242.,  20., -33., -1.,
     12., -48.,  45., -9.,
     46., -32., -27., 13.,
    -44.,  43.,   6., -5.
    );
  expected *= 1./228.;

  for(int i = 0; i < 4; ++ i)
    for(int j = 0; j < 4; ++ j)
      CATCH_CHECK(M(i,j) == Approx(expected(i,j)).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic, inverse_2x2")
{
  auto M = cml::inverse(
    cml::matrixd(
      2,2,
      1., 2.,
      3., 4.
      )
    );
  CATCH_CHECK(M(0,0) == Approx(-2.0).epsilon(1e-12));
  CATCH_CHECK(M(0,1) == Approx( 1.0).epsilon(1e-12));
  CATCH_CHECK(M(1,0) == Approx( 1.5).epsilon(1e-12));
  CATCH_CHECK(M(1,1) == Approx(-0.5).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic, size_check1")
{
  cml::matrixd M(3,4);
  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_REQUIRE_THROWS_AS(M.inverse(), cml::non_square_matrix_error);
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
