/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <cml/vector/dynamic_external.h>
#include <cml/vector/comparison.h>
#include <cml/vector/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("array_construct")
{
  double data[] = { 1., 2., 3. };
  cml::externalnd v(data, 3);
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data() == &data[0]);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("array_assign")
{
  double av[3];
  double data[] = { 1., 2., 3. };
  cml::externalnd v(av, 3);
  v = data;
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("list_assign")
{
  double av[3];
  cml::externalnd v(av, 3);
  v = { 1., 2., 3. };
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("fill1") {
  double av[5];
  cml::externalnd v(av, 5);
  v.fill(1.);
  CATCH_REQUIRE(v.size() == 5);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[4] == 1.);
}

CATCH_TEST_CASE("write1")
{
  double data[] = { 1., 2., 3. };
  cml::externalnd v(data, 3);
  CATCH_REQUIRE(v.size() == 3);
  v[0] = 1.;
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("size_check1")
{
  double av[3];
  cml::externalnd v(av, 3);
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK_THROWS_AS(
    (v = { 1., 2., 3., 4. }), cml::incompatible_vector_size_error);
}

CATCH_TEST_CASE("const1")
{
  const double av[] = { 1., 2., 3. };
  cml::externalncd v(av, 3);
  CATCH_REQUIRE(v.size() == 3);
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
