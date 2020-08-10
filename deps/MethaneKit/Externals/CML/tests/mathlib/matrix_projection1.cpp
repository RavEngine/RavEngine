/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/mathlib/matrix/projection.h>

#include <cml/vector.h>
#include <cml/matrix.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("orthographic, rh1")
{
  cml::matrix44d O;
  cml::matrix_orthographic_RH(
    O, -.5, .5, -.5, .5, -1., 1., cml::z_clip_neg_one);
  // l, r, b, t, near, far

  cml::vector4d o = O * cml::vector4d(.5, .5, 0., 1.);
  CATCH_CHECK(o[0] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(o[1] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(0 == Approx(o[2]).epsilon(0).margin(1e-7));
  CATCH_CHECK(o[3] == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("orthographic, rh2")
{
  cml::matrix44d O;
  cml::matrix_orthographic_RH(O, 1., 1., -1., 1., cml::z_clip_neg_one);
  // width x height, near, far

  cml::vector4d o = O * cml::vector4d(.5, .5, 0., 1.);
  CATCH_CHECK(o[0] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(o[1] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(0 == Approx(o[2]).epsilon(0).margin(1e-7));
  CATCH_CHECK(o[3] == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("orthographic, lh1")
{
  cml::matrix44d O;
  cml::matrix_orthographic_LH(
    O, -.5, .5, -.5, .5, -1., 1., cml::z_clip_zero);
  // l, r, b, t, near, far

  cml::vector4d o = O * cml::vector4d(.5, .5, 0., 1.);
  CATCH_CHECK(o[0] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(o[1] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(o[2] == Approx(.5).epsilon(1e-12));
  CATCH_CHECK(o[3] == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("orthographic, lh2")
{
  cml::matrix44d O;
  cml::matrix_orthographic_LH(O, 1., 1., -1., 1., cml::z_clip_zero);
  // width x height, near, far

  cml::vector4d o = O * cml::vector4d(.5, .5, 0., 1.);
  CATCH_CHECK(o[0] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(o[1] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(o[2] == Approx(.5).epsilon(1e-12));
  CATCH_CHECK(o[3] == Approx(1.).epsilon(1e-12));
}




CATCH_TEST_CASE("perspective, rh1")
{
  cml::matrix44d P;
  cml::matrix_perspective_RH(
    P, -.5, .5, -.5, .5, .001, 1., cml::z_clip_neg_one);
  // l, r, b, t, near, far

  cml::vector4d p = P * cml::vector4d(.5, .5, 1., 1.);
  CATCH_CHECK(p[0] == Approx(.001).epsilon(1e-12));
  CATCH_CHECK(p[1] == Approx(.001).epsilon(1e-12));
  CATCH_CHECK(p[2] == Approx(-1.004004004004).epsilon(1e-12));
  CATCH_CHECK(p[3] == Approx(-1.).epsilon(1e-12));
}

CATCH_TEST_CASE("perspective, rh2")
{
  cml::matrix44d P;
  cml::matrix_perspective_RH(P, 1., 1., .001, 1., cml::z_clip_neg_one);
  // width x height, near, far

  cml::vector4d p = P * cml::vector4d(.5, .5, 1., 1.);
  CATCH_CHECK(p[0] == Approx(.001).epsilon(1e-12));
  CATCH_CHECK(p[1] == Approx(.001).epsilon(1e-12));
  CATCH_CHECK(p[2] == Approx(-1.004004004004).epsilon(1e-12));
  CATCH_CHECK(p[3] == Approx(-1.).epsilon(1e-12));
}

CATCH_TEST_CASE("perspective, lh1")
{
  cml::matrix44d P;
  cml::matrix_perspective_LH(
    P, -.5, .5, -.5, .5, .001, 1., cml::z_clip_zero);
  // l, r, b, t, near, far

  cml::vector4d p = P * cml::vector4d(.5, .5, 1., 1.);
  CATCH_CHECK(p[0] == Approx(.001).epsilon(1e-12));
  CATCH_CHECK(p[1] == Approx(.001).epsilon(1e-12));
  CATCH_CHECK(p[2] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(p[3] == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("perspective, lh2")
{
  cml::matrix44d P;
  cml::matrix_perspective_LH(P, 1., 1., .001, 1., cml::z_clip_zero);
  // width x height, near, far

  cml::vector4d p = P * cml::vector4d(.5, .5, 1., 1.);
  CATCH_CHECK(p[0] == Approx(.001).epsilon(1e-12));
  CATCH_CHECK(p[1] == Approx(.001).epsilon(1e-12));
  CATCH_CHECK(p[2] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(p[3] == Approx(1.).epsilon(1e-12));
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
