/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/matrix/hadamard_product.h>

#include <cml/matrix/fixed.h>
#include <cml/matrix/external.h>
#include <cml/matrix/dynamic.h>
#include <cml/matrix/types.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("fixed, hadamard1")
{
  cml::matrix22d M1(
    1., 2.,
    3., 4.
    );
  cml::matrix22d M2(
    5., 6.,
    7., 8.
    );

  auto M = cml::hadamard(M1,M2);
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 5.);
  CATCH_CHECK(M(0,1) == 12.);
  CATCH_CHECK(M(1,0) == 21.);
  CATCH_CHECK(M(1,1) == 32.);
}


CATCH_TEST_CASE("fixed external, hadamard1")
{
  double aM1[] = {
    1., 2.,
    3., 4.
  };
  cml::external22d M1(aM1);

  double aM2[] = {
    5., 6.,
    7., 8.
  };
  cml::external22d M2(aM2);

  auto M = cml::hadamard(M1,M2);
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 5.);
  CATCH_CHECK(M(0,1) == 12.);
  CATCH_CHECK(M(1,0) == 21.);
  CATCH_CHECK(M(1,1) == 32.);
}


CATCH_TEST_CASE("dynamic external, hadamard1")
{
  double aM1[] = {
    1., 2.,
    3., 4.
  };
  cml::externalmnd M1(2,2, aM1);

  double aM2[] = {
    5., 6.,
    7., 8.
  };
  cml::externalmnd M2(2,2, aM2);

  auto M = cml::hadamard(M1,M2);
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 5.);
  CATCH_CHECK(M(0,1) == 12.);
  CATCH_CHECK(M(1,0) == 21.);
  CATCH_CHECK(M(1,1) == 32.);
}

CATCH_TEST_CASE("dynamic, hadamard1")
{
  cml::matrixd M1(2,2,
    1., 2.,
    3., 4.
    );
  cml::matrixd M2(2,2,
    5., 6.,
    7., 8.
    );

  auto M = cml::hadamard(M1,M2);
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 5.);
  CATCH_CHECK(M(0,1) == 12.);
  CATCH_CHECK(M(1,0) == 21.);
  CATCH_CHECK(M(1,1) == 32.);
}


CATCH_TEST_CASE("mixed fixed, dynamic1")
{
  cml::matrix22d M1(
    1., 2.,
    3., 4.
    );
  cml::matrixd M2(2,2,
    5., 6.,
    7., 8.
    );

  auto M = cml::hadamard(M1,M2);
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 5.);
  CATCH_CHECK(M(0,1) == 12.);
  CATCH_CHECK(M(1,0) == 21.);
  CATCH_CHECK(M(1,1) == 32.);
}

CATCH_TEST_CASE("mixed fixed, fixed external1")
{
  cml::matrix22d M1(
    1., 2.,
    3., 4.
    );

  double aM2[] = {
    5., 6.,
    7., 8.
  };
  cml::external22d M2(aM2);

  auto M = cml::hadamard(M1,M2);
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 5.);
  CATCH_CHECK(M(0,1) == 12.);
  CATCH_CHECK(M(1,0) == 21.);
  CATCH_CHECK(M(1,1) == 32.);
}

CATCH_TEST_CASE("mixed fixed, dynamic external1")
{
  cml::matrix22d M1(
    1., 2.,
    3., 4.
    );

  double aM2[] = {
    5., 6.,
    7., 8.
  };
  cml::externalmnd M2(2,2, aM2);

  auto M = cml::hadamard(M1,M2);
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 5.);
  CATCH_CHECK(M(0,1) == 12.);
  CATCH_CHECK(M(1,0) == 21.);
  CATCH_CHECK(M(1,1) == 32.);
}


CATCH_TEST_CASE("mixed dynamic, fixed external1")
{
  double aM1[] = {
    1., 2.,
    3., 4.
  };
  cml::externalmnd M1(2,2, aM1);

  double aM2[] = {
    5., 6.,
    7., 8.
  };
  cml::external22d M2(aM2);

  auto M = cml::hadamard(M1,M2);
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 5.);
  CATCH_CHECK(M(0,1) == 12.);
  CATCH_CHECK(M(1,0) == 21.);
  CATCH_CHECK(M(1,1) == 32.);
}

CATCH_TEST_CASE("mixed dynamic, dynamic external1")
{
  double aM1[] = {
    1., 2.,
    3., 4.
  };
  cml::externalmnd M1(2,2, aM1);

  double aM2[] = {
    5., 6.,
    7., 8.
  };
  cml::externalmnd M2(2,2,aM2);

  auto M = cml::hadamard(M1,M2);
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 5.);
  CATCH_CHECK(M(0,1) == 12.);
  CATCH_CHECK(M(1,0) == 21.);
  CATCH_CHECK(M(1,1) == 32.);
}

CATCH_TEST_CASE("mixed fixed external, dynamic external1")
{
  double aM1[] = {
    1., 2.,
    3., 4.
  };
  cml::external22d M1(aM1);

  double aM2[] = {
    5., 6.,
    7., 8.
  };
  cml::externalmnd M2(2,2, aM2);

  auto M = cml::hadamard(M1,M2);
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 5.);
  CATCH_CHECK(M(0,1) == 12.);
  CATCH_CHECK(M(1,0) == 21.);
  CATCH_CHECK(M(1,1) == 32.);
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
