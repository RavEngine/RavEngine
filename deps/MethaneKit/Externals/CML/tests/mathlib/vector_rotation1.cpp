/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/mathlib/vector/rotation.h>

#include <cml/vector.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("rotation 3D, world_axis1")
{
  auto vx = cml::rotate_vector(
    cml::vector3d(0., 1., 0.), cml::vector3d(1., 0., 0.), cml::rad(90.));
  CATCH_CHECK(0 == Approx(vx[0]).epsilon(0).margin(1e-7));
  CATCH_CHECK(0 == Approx(vx[1]).epsilon(0).margin(1e-7));
  CATCH_CHECK(vx[2] == Approx(1.).epsilon(1e-12));

  auto vy = cml::rotate_vector(
    cml::vector3d(-1., 0., 0.), cml::vector3d(0., 1., 0.), cml::rad(90.));
  CATCH_CHECK(0 == Approx(vy[0]).epsilon(0).margin(1e-7));
  CATCH_CHECK(0 == Approx(vy[1]).epsilon(0).margin(1e-7));
  CATCH_CHECK(vy[2] == Approx(1.).epsilon(1e-12));

  auto vz = cml::rotate_vector(
    cml::vector3d(1., 0., 0.), cml::vector3d(0., 0., 1.), cml::rad(90.));
  CATCH_CHECK(0 == Approx(vz[0]).epsilon(0).margin(1e-7));
  CATCH_CHECK(vz[1] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(0 == Approx(vz[2]).epsilon(0).margin(1e-7));
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
