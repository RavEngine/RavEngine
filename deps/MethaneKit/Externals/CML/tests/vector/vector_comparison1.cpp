/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/vector/comparison.h>

#include <cml/vector/fixed.h>
#include <cml/vector/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("less_greater1")
{
  cml::vector3d v = { 1., 2., 3. };
  cml::vector3d w = { 2., 2., 2. };
  CATCH_REQUIRE(v < w);
  CATCH_REQUIRE(w > v);
}

CATCH_TEST_CASE("less_greater2")
{
  cml::vector3d v = { 1., 2., 3. };
  cml::vector3d w = { 1., 2., 2. };
  CATCH_REQUIRE(w < v);
  CATCH_REQUIRE(v > w);
}

CATCH_TEST_CASE("less_greater3")
{
  cml::vector3d v = { 1., 2., 3. };
  cml::vector3d w = { 1., 2., 3. };
  CATCH_REQUIRE(!(w < v));
  CATCH_REQUIRE(!(v < w));
}

CATCH_TEST_CASE("less_equal1")
{
  cml::vector3d v = { 1., 2., 3. };
  cml::vector3d w = { 1., 2., 3. };
  CATCH_REQUIRE(v <= w);
  CATCH_REQUIRE(w <= v);
}

CATCH_TEST_CASE("less_equal2")
{
  cml::vector3d v = { 1., 2., 3. };
  cml::vector3d w = { 1., 2., 4. };
  CATCH_REQUIRE(v <= w);
  CATCH_REQUIRE(!(w <= v));
}

CATCH_TEST_CASE("greater_equal1")
{
  cml::vector3d v = { 1., 2., 3. };
  cml::vector3d w = { 1., 2., 3. };
  CATCH_REQUIRE(v >= w);
  CATCH_REQUIRE(w >= v);
}

CATCH_TEST_CASE("greater_equal2")
{
  cml::vector3d v = { 1., 2., 3. };
  cml::vector3d w = { 1., 2., 4. };
  CATCH_REQUIRE(w >= v);
  CATCH_REQUIRE(!(v >= w));
}

CATCH_TEST_CASE("equal1")
{
  cml::vector3d v = { 1., 2., 3. };
  cml::vector3d w = { 1., 2., 3. };
  CATCH_REQUIRE(w == v);
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
