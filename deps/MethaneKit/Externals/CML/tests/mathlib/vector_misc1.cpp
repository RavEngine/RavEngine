/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <iostream>

// Make sure the main header compiles cleanly:
#include <cml/mathlib/vector/misc.h>

#include <cml/vector.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("project_to_hplane1")
{
  auto v = cml::vector3d(1., 1., 1.);
  auto n = cml::vector3d(0., 0., 1.);
  auto p = cml::project_to_hplane(v, n);
  CATCH_CHECK(p[0] == 1.);
  CATCH_CHECK(p[1] == 1.);
  CATCH_CHECK(p[2] == 0.);
}

CATCH_TEST_CASE("perp1")
{
  auto v = cml::vector2d(1., 1.);
  auto p = cml::perp(v);
  CATCH_CHECK(p[0] == -1.);
  CATCH_CHECK(p[1] == 1.);
}

CATCH_TEST_CASE("city_block1")
{
  auto v1 = cml::vector3d(3., 7., 1.);
  auto v2 = cml::vector3d(1., 9., 2.);
  auto c = cml::manhattan_distance(v1, v2);
  CATCH_CHECK(c == Approx(5.).epsilon(1e-12));
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
