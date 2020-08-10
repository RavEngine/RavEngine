/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <iostream>

// Make sure the main header compiles cleanly:
#include <cml/matrix/determinant.h>

#include <cml/matrix/fixed.h>
#include <cml/matrix/dynamic.h>
#include <cml/matrix/external.h>
#include <cml/matrix/types.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("fixed, det_2x2")
{
  auto M = cml::matrix22d(
     2.,  -1.,
     3.,  3.
    );
  auto result = cml::determinant(M);
  CATCH_CHECK(result == Approx(9).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, det_3x3")
{
  auto M = cml::matrix33d(
     2.,  0.,  2.,
     3.,  3.,  4.,
     5.,  5.,  4.
    );
  auto result = cml::determinant(M);
  CATCH_CHECK(result == Approx(-16).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, det_4x4")
{
  auto M = cml::matrix44d(
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
    );
  auto result = cml::determinant(M);
  CATCH_CHECK(result == Approx(-120).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, det_5x5")
{
  auto M = cml::matrix<double, cml::fixed<5,5>>(
     2.,  0.,  2.,  .5,   2.,
     3.,  3.,  4.,  -2., -1.,
     5.,  5.,  4.,   2.,  1.,
    -1., -2.,  3.,  -1.,  5.,
     1.,  6.,  7.,  .5,   1.
    );
  auto result = cml::determinant(M);
  CATCH_CHECK(result == Approx(464).epsilon(1e-12));
}




CATCH_TEST_CASE("fixed external, det_2x2")
{
  double avM[] = {
    2.,  -1.,
    3.,  3.
  };
  auto M = cml::external22d(avM);
  auto result = cml::determinant(M);
  CATCH_CHECK(result == Approx(9).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed external, det_3x3")
{
  double avM[] = {
     2.,  0.,  2.,
     3.,  3.,  4.,
     5.,  5.,  4.
  };
  auto M = cml::external33d(avM);
  auto result = cml::determinant(M);
  CATCH_CHECK(result == Approx(-16).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed external, det_4x4")
{
  double avM[] = {
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
  };
  auto M = cml::external44d(avM);
  auto result = cml::determinant(M);
  CATCH_CHECK(result == Approx(-120).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed external, det_5x5")
{
  double avM[] = {
     2.,  0.,  2.,  .5,   2.,
     3.,  3.,  4.,  -2., -1.,
     5.,  5.,  4.,   2.,  1.,
    -1., -2.,  3.,  -1.,  5.,
     1.,  6.,  7.,  .5,   1.
  };
  auto M = cml::matrix<double, cml::external<5,5>>(avM);
  auto result = cml::determinant(M);
  CATCH_CHECK(result == Approx(464).epsilon(1e-12));
}




CATCH_TEST_CASE("dynamic external, det_2x2")
{
  double avM[] = {
    2.,  -1.,
    3.,  3.
  };
  auto M = cml::externalmnd(2,2, avM);
  auto result = cml::determinant(M);
  CATCH_CHECK(result == Approx(9).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic external, det_3x3")
{
  double avM[] = {
     2.,  0.,  2.,
     3.,  3.,  4.,
     5.,  5.,  4.
  };
  auto M = cml::externalmnd(3,3, avM);
  auto result = cml::determinant(M);
  CATCH_CHECK(result == Approx(-16).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic external, det_4x4")
{
  double avM[] = {
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
  };
  auto M = cml::externalmnd(4,4, avM);
  auto result = cml::determinant(M);
  CATCH_CHECK(result == Approx(-120).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic external, det_5x5")
{
  double avM[] = {
     2.,  0.,  2.,  .5,   2.,
     3.,  3.,  4.,  -2., -1.,
     5.,  5.,  4.,   2.,  1.,
    -1., -2.,  3.,  -1.,  5.,
     1.,  6.,  7.,  .5,   1.
  };
  auto M = cml::matrix<double, cml::external<>>(5,5, avM);
  auto result = cml::determinant(M);
  CATCH_CHECK(result == Approx(464).epsilon(1e-12));
}




CATCH_TEST_CASE("dynamic, det_2x2")
{
  auto M = cml::matrixd(
    2,2,
     2.,  -1.,
     3.,  3.
    );
  auto result = cml::determinant(M);
  CATCH_CHECK(result == Approx(9).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic, det_3x3")
{
  auto M = cml::matrixd(
    3,3,
     2.,  0.,  2.,
     3.,  3.,  4.,
     5.,  5.,  4.
    );
  auto result = cml::determinant(M);
  CATCH_CHECK(result == Approx(-16).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic, det_4x4")
{
  auto M = cml::matrixd(
    4,4,
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
    );
  auto result = cml::determinant(M);
  CATCH_CHECK(result == Approx(-120).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic, det_5x5")
{
  auto M = cml::matrix<double, cml::dynamic<>>(
    5,5,
     2.,  0.,  2.,  .5,   2.,
     3.,  3.,  4.,  -2., -1.,
     5.,  5.,  4.,   2.,  1.,
    -1., -2.,  3.,  -1.,  5.,
     1.,  6.,  7.,  .5,   1.
    );
  auto result = cml::determinant(M);
  CATCH_CHECK(result == Approx(464).epsilon(1e-12));
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
