/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <cml/vector/fixed.h>
#include <cml/vector/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("array_construct")
{
  double data[] = { 1., 2., 3. };
  cml::vector3d v(data);
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("array_temp_construct")
{
  double data[] = { 1., 2., 3. };
  cml::vector3d v = data;
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("element_construct1")
{
  cml::vector<double, cml::fixed<1>> v(1.);
  CATCH_REQUIRE(v.size() == 1);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("element_construct2")
{
  cml::vector2d v(1., 2.);
  CATCH_REQUIRE(v.size() == 2);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
  CATCH_CHECK(v[1] == 2.);
}

CATCH_TEST_CASE("element_construct3")
{
  cml::vector3d v(1., 2., 3.);
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
  CATCH_CHECK(v[1] == 2.);
  CATCH_CHECK(v[2] == 3.);
}

CATCH_TEST_CASE("element_construct4")
{
  cml::vector4d v(1.,2.f,3,4U);
  CATCH_REQUIRE(v.size() == 4);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
  CATCH_CHECK(v[1] == 2.);
  CATCH_CHECK(v[2] == 3.);
  CATCH_CHECK(v[3] == 4.);
}

CATCH_TEST_CASE("combine_construct1")
{
  cml::vector4d v(cml::vector3d(1., 2., 3.), 4.);
  CATCH_REQUIRE(v.size() == 4);
  CATCH_CHECK(v[0] == 1.);
  CATCH_CHECK(v[1] == 2.);
  CATCH_CHECK(v[2] == 3.);
  CATCH_CHECK(v[3] == 4.);
}

CATCH_TEST_CASE("combine_construct2")
{
  cml::vector4d v(cml::vector2d(1., 2.), 3., 4.);
  CATCH_REQUIRE(v.size() == 4);
  CATCH_CHECK(v[0] == 1.);
  CATCH_CHECK(v[1] == 2.);
  CATCH_CHECK(v[2] == 3.);
  CATCH_CHECK(v[3] == 4.);
}

CATCH_TEST_CASE("element_set1")
{
  cml::vector<double, cml::fixed<1>> v;
  CATCH_REQUIRE(v.size() == 1);
  v.set(1.);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("element_set2")
{
  cml::vector2d v;
  CATCH_REQUIRE(v.size() == 2);
  v.set(1., 2.);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
  CATCH_CHECK(v[1] == 2.);
}

CATCH_TEST_CASE("element_set3")
{
  cml::vector3d v;
  CATCH_REQUIRE(v.size() == 3);
  v.set(1., 2., 3.);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
  CATCH_CHECK(v[1] == 2.);
  CATCH_CHECK(v[2] == 3.);
}

CATCH_TEST_CASE("element_set4")
{
  cml::vector4d v;
  CATCH_REQUIRE(v.size() == 4);
  v.set(1.,2.f,3,4U);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
  CATCH_CHECK(v[1] == 2.);
  CATCH_CHECK(v[2] == 3.);
  CATCH_CHECK(v[3] == 4.);
}

CATCH_TEST_CASE("array_assign")
{
  double data[] = { 1., 2., 3. };
  cml::vector3d v;
  v = data;
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("pointer_construct")
{
  double data[] = { 1., 2., 3. };
  cml::vector3d v(&data[0]);
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("list_construct")
{
  cml::vector3d v { 1., 2., 3. };
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("list_temp_construct")
{
  cml::vector3d v = { 1., 2., 3. };
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("list_assign")
{
  cml::vector3d v;
  v = { 1., 2., 3. };
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("fill1") {
  cml::vector3d v;
  v.fill(1.);
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[2] == 1.);
}

CATCH_TEST_CASE("write1")
{
  cml::vector3d v;
  CATCH_REQUIRE(v.size() == 3);
  v[0] = 1.;
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("size_check1")
{
  cml::vector3d v;
  CATCH_REQUIRE(v.size() == 3);
  CATCH_REQUIRE_THROWS_AS(
    (v = { 1., 2., 3., 4. }), cml::incompatible_vector_size_error);
}

#ifdef CML_HAS_STRUCTURED_BINDINGS
CATCH_TEST_CASE("structured_binding1")
{
  auto [x,y,z] = cml::vector3d(1., 2., 3. );
  CATCH_CHECK(x == 1.);
  CATCH_CHECK(y == 2.);
  CATCH_CHECK(z == 3.);
}
#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
