/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/vector/dot.h>

#include <cml/vector/fixed.h>
#include <cml/vector/dynamic.h>
#include <cml/vector/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("dot1")
{
  cml::vector3d v1 = { 1., 1., 1. };
  cml::vector3d v2 = { 2., 2., 2. };
  double dp = cml::dot(v1,v2);
  CATCH_CHECK(dp == 6.);
}

CATCH_TEST_CASE("size_check1")
{
  cml::vectord v1;
  CATCH_CHECK_THROWS_AS(cml::dot(v1,v1), cml::minimum_vector_size_error);
}

CATCH_TEST_CASE("size_check2")
{
  cml::vectord v1(2,3);
  cml::vector3d v2(2,3,3);
  CATCH_CHECK_THROWS_AS(cml::dot(v1,v2), cml::incompatible_vector_size_error);
}

CATCH_TEST_CASE("size_check3")
{
  cml::vector3d v1(2,3,3);
  cml::vectord v2(2,3);
  CATCH_CHECK_THROWS_AS(cml::dot(v1,v2), cml::incompatible_vector_size_error);
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
