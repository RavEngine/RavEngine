/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <cml/vector/fixed_external.h>
#include <cml/vector/comparison.h>
#include <cml/vector/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("array_construct")
{
  double data[] = { 1., 2., 3. };
  cml::external3d v(data);
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data() == &data[0]);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("array_assign")
{
  double av[3];
  double data[] = { 1., 2., 3. };
  cml::external3d v(av);
  v = data;
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("list_assign")
{
  double av[3];
  cml::external3d v(av);
  v = { 1., 2., 3. };
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("fill1") {
  double av[3];
  cml::external3d v(av);
  v.fill(1.);
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[2] == 1.);
}

CATCH_TEST_CASE("write1")
{
  double data[] = { 1., 2., 3. };
  cml::external3d v(data);
  CATCH_REQUIRE(v.size() == 3);
  v[0] = 1.;
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("size_check1")
{
  double av[3];
  cml::external3d v(av);
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK_THROWS_AS(
    (v = { 1., 2., 3., 4. }), cml::incompatible_vector_size_error);
}

CATCH_TEST_CASE("const1")
{
  const double av[] = { 1., 2., 3. };
  cml::external3cd v(av);
  CATCH_REQUIRE(v.size() == 3);
}

#ifdef CML_HAS_STRUCTURED_BINDINGS
CATCH_TEST_CASE("structured_binding1")
{
  double data[] = { 1., 2., 3. };
  auto [x,y,z] = cml::external3d(data);
  CATCH_CHECK(x == 1.);
  CATCH_CHECK(y == 2.);
  CATCH_CHECK(z == 3.);
}
#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
