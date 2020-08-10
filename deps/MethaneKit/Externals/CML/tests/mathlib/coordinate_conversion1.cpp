/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/mathlib/coordinate_conversion.h>

#include <cml/vector.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("from cartesian, to_polar1")
{
  cml::vector2d x;
  cml::polar_to_cartesian(x, 1., cml::rad(45.));
  CATCH_CHECK(x[0] == Approx(.7071067811865).epsilon(1e-12));
  CATCH_CHECK(x[1] == Approx(.7071067811865).epsilon(1e-12));
}

CATCH_TEST_CASE("from cartesian, to_polar2")
{
  cml::vector2d x;
  cml::polar_to_cartesian(1., cml::rad(45.), x);
  CATCH_CHECK(x[0] == Approx(.7071067811865).epsilon(1e-12));
  CATCH_CHECK(x[1] == Approx(.7071067811865).epsilon(1e-12));
}


CATCH_TEST_CASE("from cartesian, to_cylindrical1")
{
  cml::vector3d x;
  cml::cylindrical_to_cartesian(x, 2, 1., cml::rad(45.), 1.);
  CATCH_CHECK(x[0] == Approx(.7071067811865).epsilon(1e-12));
  CATCH_CHECK(x[1] == Approx(.7071067811865).epsilon(1e-12));
  CATCH_CHECK(x[2] == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("from cartesian, to_cylindrical2")
{
  cml::vector3d x;
  cml::cylindrical_to_cartesian(1., cml::rad(45.), 1., 2, x);
  CATCH_CHECK(x[0] == Approx(.7071067811865).epsilon(1e-12));
  CATCH_CHECK(x[1] == Approx(.7071067811865).epsilon(1e-12));
  CATCH_CHECK(x[2] == Approx(1.).epsilon(1e-12));
}


CATCH_TEST_CASE("from cartesian, to_spherical1")
{
  cml::vector3d x;
  cml::spherical_to_cartesian(
    x, 2, cml::colatitude, 1., cml::rad(45.), cml::rad(45.));
  CATCH_CHECK(x[0] == Approx(.5).epsilon(1e-12));
  CATCH_CHECK(x[1] == Approx(.5).epsilon(1e-12));
  CATCH_CHECK(x[2] == Approx(.7071067811865).epsilon(1e-12));
}

CATCH_TEST_CASE("from cartesian, to_spherical2")
{
  cml::vector3d x;
  cml::spherical_to_cartesian(
    1., cml::rad(45.), cml::rad(45.), 2, cml::colatitude, x);
  CATCH_CHECK(x[0] == Approx(.5).epsilon(1e-12));
  CATCH_CHECK(x[1] == Approx(.5).epsilon(1e-12));
  CATCH_CHECK(x[2] == Approx(.7071067811865).epsilon(1e-12));
}




CATCH_TEST_CASE("to cartesian, from_polar1")
{
  cml::vector2d x(std::sqrt(.5), std::sqrt(.5));
  double radius, theta;
  cml::cartesian_to_polar(x, radius, theta);
  CATCH_CHECK(radius == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(theta == Approx(cml::rad(45.)).epsilon(1e-12));
}

CATCH_TEST_CASE("to cartesian, from_polar2")
{
  cml::vector2d x(std::sqrt(.5), std::sqrt(.5));
  double radius, theta, tolerance = 1e-7;
  cml::cartesian_to_polar(x, radius, theta, tolerance);
  CATCH_CHECK(radius == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(theta == Approx(cml::rad(45.)).epsilon(1e-12));
}


CATCH_TEST_CASE("to cartesian, from_cylindrical1")
{
  cml::vector3d x(std::sqrt(.5), std::sqrt(.5), 1.);
  double radius, theta, height;
  cml::cartesian_to_cylindrical(x, 2, radius, theta, height);
  CATCH_CHECK(radius == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(theta == Approx(cml::rad(45.)).epsilon(1e-12));
  CATCH_CHECK(height == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("to cartesian, from_cylindrical2")
{
  cml::vector3d x(std::sqrt(.5), std::sqrt(.5), 1.);
  double radius, theta, height, tolerance = 1e-7;
  cml::cartesian_to_cylindrical(x, 2, radius, theta, height, tolerance);
  CATCH_CHECK(radius == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(theta == Approx(cml::rad(45.)).epsilon(1e-12));
  CATCH_CHECK(height == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("to cartesian, from_cylindrical3")
{
  cml::vector3d x(std::sqrt(.5), std::sqrt(.5), 1.);
  double radius, theta, height;
  cml::cartesian_to_cylindrical(x, radius, theta, height, 2);
  CATCH_CHECK(radius == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(theta == Approx(cml::rad(45.)).epsilon(1e-12));
  CATCH_CHECK(height == Approx(1.).epsilon(1e-12));
}


CATCH_TEST_CASE("to cartesian, from_spherical1")
{
  cml::vector3d x(.5, .5, std::sqrt(.5));
  double radius, theta, phi;
  cml::cartesian_to_spherical(x, 2, cml::colatitude, radius, theta, phi);
  CATCH_CHECK(radius == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(theta == Approx(cml::rad(45.)).epsilon(1e-12));
  CATCH_CHECK(phi == Approx(cml::rad(45.)).epsilon(1e-12));
}

CATCH_TEST_CASE("to cartesian, from_spherical2")
{
  cml::vector3d x(.5, .5, std::sqrt(.5));
  double radius, theta, phi, tolerance = 1e-7;
  cml::cartesian_to_spherical(
    x, 2, cml::colatitude, radius, theta, phi, tolerance);
  CATCH_CHECK(radius == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(theta == Approx(cml::rad(45.)).epsilon(1e-12));
  CATCH_CHECK(phi == Approx(cml::rad(45.)).epsilon(1e-12));
}

CATCH_TEST_CASE("to cartesian, from_spherical3")
{
  cml::vector3d x(.5, .5, std::sqrt(.5));
  double radius, theta, phi;
  cml::cartesian_to_spherical(
    x, radius, theta, phi, 2, cml::colatitude);
  CATCH_CHECK(radius == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(theta == Approx(cml::rad(45.)).epsilon(1e-12));
  CATCH_CHECK(phi == Approx(cml::rad(45.)).epsilon(1e-12));
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
