/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <iostream>

// Make sure the main header compiles cleanly:
#include <cml/quaternion/comparison.h>

#include <cml/quaternion/fixed.h>
#include <cml/quaternion/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("less_greater1")
{
  cml::quaterniond v = { 1., 2., 3., 4. };
  cml::quaterniond w = { 2., 2., 2., 2. };
  CATCH_REQUIRE(v < w);
  CATCH_REQUIRE(w > v);
}

CATCH_TEST_CASE("less_greater2")
{
  cml::quaterniond v = { 1., 2., 3., 4. };
  cml::quaterniond w = { 1., 2., 2., 4. };
  CATCH_REQUIRE(w < v);
  CATCH_REQUIRE(v > w);
}

CATCH_TEST_CASE("less_greater3")
{
  cml::quaterniond v = { 1., 2., 3., 4. };
  cml::quaterniond w = { 1., 2., 3., 4. };
  CATCH_REQUIRE(!(w < v));
  CATCH_REQUIRE(!(v < w));
}

CATCH_TEST_CASE("less_equal1")
{
  cml::quaterniond v = { 1., 2., 3., 4. };
  cml::quaterniond w = { 1., 2., 3., 4. };
  CATCH_REQUIRE(v <= w);
  CATCH_REQUIRE(w <= v);
}

CATCH_TEST_CASE("less_equal2")
{
  cml::quaterniond v = { 1., 2., 3., 4. };
  cml::quaterniond w = { 1., 2., 4., 4. };
  CATCH_REQUIRE(v <= w);
  CATCH_REQUIRE(!(w <= v));
}

CATCH_TEST_CASE("greater_equal1")
{
  cml::quaterniond v = { 1., 2., 3., 4. };
  cml::quaterniond w = { 1., 2., 3., 4. };
  CATCH_REQUIRE(v >= w);
  CATCH_REQUIRE(w >= v);
}

CATCH_TEST_CASE("greater_equal2")
{
  cml::quaterniond v = { 1., 2., 3., 4. };
  cml::quaterniond w = { 1., 2., 4., 4. };
  CATCH_REQUIRE(w >= v);
  CATCH_REQUIRE(!(v >= w));
}

CATCH_TEST_CASE("equal1")
{
  cml::quaterniond v = { 1., 2., 3., 4. };
  cml::quaterniond w = { 1., 2., 3., 4. };
  CATCH_REQUIRE(w == v);
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
