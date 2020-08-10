/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <iostream>

// Make sure the main header compiles cleanly:
#include <cml/mathlib/random_unit.h>

#include <cml/vector.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("random_unit1")
{
  cml::vector3d n;
  cml::random_unit(n);
  CATCH_CHECK(n.length_squared() == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("random_unit2")
{
  std::random_device rd;
  std::mt19937 gen(rd());
  cml::vector3d n;
  cml::random_unit(n, gen);
  CATCH_CHECK(n.length_squared() == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("random_cone1")
{
  cml::vector2d n, d(1.,0.);
  cml::random_unit(n, d, cml::rad(15.));
  CATCH_CHECK(n.length_squared() == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("random_cone2")
{
  cml::vector3d n, d(0.,0.,1.);
  cml::random_unit(n, d, cml::rad(30.));
  CATCH_CHECK(n.length_squared() == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("random_cone3")
{
  cml::vector4d n, d(0.,0.,0.,1.);
  cml::random_unit(n, d, cml::rad(60.));
  CATCH_CHECK(n.length_squared() == Approx(1.).epsilon(1e-12));
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
