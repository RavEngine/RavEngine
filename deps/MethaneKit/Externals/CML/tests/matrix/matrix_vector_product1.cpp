/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <typeinfo>
#include <iostream>

// Make sure the main header compiles cleanly:
#include <cml/matrix/vector_product.h>

#include <cml/vector/fixed.h>
#include <cml/vector/external.h>
#include <cml/vector/dynamic.h>
#include <cml/matrix/fixed.h>
#include <cml/matrix/external.h>
#include <cml/matrix/dynamic.h>
#include <cml/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("fixed product1")
{
  cml::matrix22d M(
    1., 2.,
    3., 4.
    );
  cml::vector2d v1(
    5., 6.
    );

  auto v = M*v1;
  CATCH_REQUIRE((std::is_same<decltype(v), cml::vector2d>::value));
  CATCH_REQUIRE(v.size() == 2);
  CATCH_CHECK(v[0] == 17.);
  CATCH_CHECK(v[1] == 39.);
}

CATCH_TEST_CASE("fixed product2")
{
  cml::matrix22d M(
    1., 2.,
    3., 4.
    );
  cml::vector2d v1(
    5., 6.
    );

  auto v = v1*M;
  CATCH_REQUIRE((std::is_same<decltype(v), cml::vector2d>::value));
  CATCH_REQUIRE(v.size() == 2);
  CATCH_CHECK(v[0] == 23.);
  CATCH_CHECK(v[1] == 34.);
}

CATCH_TEST_CASE("fixed external product1")
{
  double aM[] = { 1., 2., 3., 4. };
  cml::external22d M(aM);

  double av1[] = { 5., 6. };
  cml::external2d v1(av1);

  auto v = M*v1;
  CATCH_REQUIRE((std::is_same<decltype(v), cml::vector2d>::value));
  CATCH_REQUIRE(v.size() == 2);
  CATCH_CHECK(v[0] == 17.);
  CATCH_CHECK(v[1] == 39.);
}

CATCH_TEST_CASE("fixed external product2")
{
  double aM[] = { 1., 2., 3., 4. };
  cml::external22d M(aM);

  double av1[] = { 5., 6. };
  cml::external2d v1(av1);

  auto v = v1*M;
  CATCH_REQUIRE((std::is_same<decltype(v), cml::vector2d>::value));
  CATCH_REQUIRE(v.size() == 2);
  CATCH_CHECK(v[0] == 23.);
  CATCH_CHECK(v[1] == 34.);
}

CATCH_TEST_CASE("dynamic external product1")
{
  double aM[] = { 1., 2., 3., 4. };
  cml::externalmnd M(aM, 2,2);

  double av1[] = { 5., 6. };
  cml::externalnd v1(2, av1);

  auto v = M*v1;
  CATCH_REQUIRE((std::is_same<decltype(v), cml::vectord>::value));
  CATCH_REQUIRE(v.size() == 2);
  CATCH_CHECK(v[0] == 17.);
  CATCH_CHECK(v[1] == 39.);
}

CATCH_TEST_CASE("dynamic external product2")
{
  double aM[] = { 1., 2., 3., 4. };
  cml::externalmnd M(aM, 2,2);

  double av1[] = { 5., 6. };
  cml::externalnd v1(2, av1);

  auto v = v1*M;
  CATCH_REQUIRE((std::is_same<decltype(v), cml::vectord>::value));
  CATCH_REQUIRE(v.size() == 2);
  CATCH_CHECK(v[0] == 23.);
  CATCH_CHECK(v[1] == 34.);
}

CATCH_TEST_CASE("dynamic external size_checking1")
{
  double aM1[4], av1[3];
  CATCH_REQUIRE_THROWS_AS(
    (cml::externalmnd(aM1, 2,2) * cml::externalnd(3, av1)),
    cml::incompatible_matrix_inner_size_error);
}

CATCH_TEST_CASE("dynamic external size_checking2")
{
  double aM1[4], av1[3];
  CATCH_REQUIRE_THROWS_AS(
    (cml::externalnd(3, av1) * cml::externalmnd(2,2, aM1)),
    cml::incompatible_matrix_inner_size_error);
}

CATCH_TEST_CASE("dynamic product1")
{
  cml::matrixd M(
    2,2,
    1., 2.,
    3., 4.
    );
  cml::vectord v1(
    5., 6.
    );

  auto v = M*v1;
  CATCH_REQUIRE((std::is_same<decltype(v), cml::vectord>::value));
  CATCH_REQUIRE(v.size() == 2);
  CATCH_CHECK(v[0] == 17.);
  CATCH_CHECK(v[1] == 39.);
}

CATCH_TEST_CASE("dynamic product2")
{
  cml::matrixd M(
    2,2,
    1., 2.,
    3., 4.
    );
  cml::vectord v1(
    5., 6.
    );

  auto v = v1*M;
  CATCH_REQUIRE((std::is_same<decltype(v), cml::vectord>::value));
  CATCH_REQUIRE(v.size() == 2);
  CATCH_CHECK(v[0] == 23.);
  CATCH_CHECK(v[1] == 34.);
}

CATCH_TEST_CASE("dynamic size_checking1")
{
  CATCH_REQUIRE_THROWS_AS(
    (cml::matrixd(2,2) * cml::vectord(3)),
    cml::incompatible_matrix_inner_size_error);
}

CATCH_TEST_CASE("dynamic size_checking2")
{
  CATCH_REQUIRE_THROWS_AS(
    (cml::vectord(3) * cml::matrixd(2,2)),
    cml::incompatible_matrix_inner_size_error);
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
