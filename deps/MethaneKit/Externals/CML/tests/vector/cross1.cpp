/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/vector/cross_node.h>

#include <cml/vector/fixed.h>
#include <cml/vector/dynamic.h>
#include <cml/vector/cross.h>
#include <cml/vector/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("cross1")
{
  cml::vector3d v1 = { 1., 1., 1. };
  cml::vector3d v2 = { 2., 2., 2. };

  typedef decltype(cml::cross(v1,v2)) cross_type;
  static_assert(
    std::is_same<cross_type::size_tag,cml::fixed_size_tag>::value,
    "got wrong size_tag from cross()");

  auto v = cml::cross(v1,v2);
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v[0] == 0.);
  CATCH_CHECK(v[1] == 0.);
  CATCH_CHECK(v[2] == 0.);
}

CATCH_TEST_CASE("cross2")
{
  cml::vectord v1(1., 2., 3.);
  cml::vector3d v2 (3., 2., 1.);

  typedef decltype(cml::cross(v1,v2)) cross_type;
  static_assert(
    std::is_same<cross_type::size_tag,cml::fixed_size_tag>::value,
    "got wrong size_tag from cross()");

  auto v = cml::cross(v1,v2);
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v[0] == -4.);
  CATCH_CHECK(v[1] ==  8.);
  CATCH_CHECK(v[2] == -4.);
}

CATCH_TEST_CASE("size_check1")
{
  cml::vectord v1(2);
  CATCH_CHECK_THROWS_AS(cml::cross(v1,v1), cml::vector_size_error);
}

CATCH_TEST_CASE("size_check2")
{
  cml::vectord v1(2,3);
  cml::vector3d v2(2,3,3);
  CATCH_CHECK_THROWS_AS(cml::cross(v1,v2), cml::vector_size_error);
}

CATCH_TEST_CASE("size_check3")
{
  cml::vector3d v1(2,3,3);
  cml::vectord v2(2,3);
  CATCH_CHECK_THROWS_AS(cml::cross(v1,v2), cml::vector_size_error);
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
