/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/mathlib/matrix/transform.h>

#include <cml/vector.h>
#include <cml/matrix.h>
#include <cml/mathlib/vector/transform.h>
#include <cml/mathlib/matrix/generators.h>

#include <cml/util.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("look at 3D, look_at_rh_1")
{
  auto M = cml::zero_4x4();
  cml::matrix_look_at_RH(M,
    cml::vector3d(1.,0.,0.),
    cml::vector3d(0.,0.,0.),
    cml::vector3d(0.,0.,1.));

  auto v = cml::vector3d(1., 1., 1.);
  auto w = cml::transform_point(M,v);

  CATCH_CHECK(w[0] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(w[1] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(0 == Approx(w[2]).epsilon(0).margin(1e-8));
}

CATCH_TEST_CASE("look at 3D, look_at_lh_1")
{
  auto M = cml::zero_4x4();
  cml::matrix_look_at_LH(M,
    cml::vector3d(1.,0.,0.),
    cml::vector3d(0.,0.,0.),
    cml::vector3d(0.,0.,1.));

  auto v = cml::vector3d(1., 1., 1.);
  auto w = cml::transform_point(M,v);

  CATCH_CHECK(w[0] == Approx(-1.).epsilon(1e-12));
  CATCH_CHECK(w[1] == Approx( 1.).epsilon(1e-12));
  CATCH_CHECK(0 == Approx(w[2]).epsilon(0).margin( 1e-8));
}




CATCH_TEST_CASE("affine 3D, test1")
{
  cml::matrix44d M;
  auto axis = cml::vector3d(1.,1.,1.).normalize();
  cml::vector3d xlate(2.,2.,2.);
  cml::matrix_affine_transform(M, axis, cml::rad(22.5), xlate);

  CATCH_CHECK(M(0,3) == 2.0);
  CATCH_CHECK(M(1,3) == 2.0);
  CATCH_CHECK(M(2,3) == 2.0);
  CATCH_CHECK(M(3,3) == 1.0);

  CATCH_CHECK(M(0,0) == Approx(0.9492530216742).epsilon(1e-12));
  CATCH_CHECK(M(1,1) == Approx(0.9492530216742).epsilon(1e-12));
  CATCH_CHECK(M(2,2) == Approx(0.9492530216742).epsilon(1e-12));
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
