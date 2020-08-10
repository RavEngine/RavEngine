/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/mathlib/vector/transform.h>

#include <cml/vector.h>
#include <cml/matrix.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("transform 2D, vector1")
{
  auto v = cml::vector2d(1., 1.);
  auto M = cml::matrix22d(
    1., 2.,
    0., 1.);
  auto w = cml::transform_vector_2D(M,v);
  CATCH_CHECK(w[0] == Approx(3.).epsilon(1e-12));
  CATCH_CHECK(w[1] == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("transform 2D, vector2")
{
  auto v = cml::vector2d(1., 1.);
  auto M = cml::matrix22d_r(
    1., 2.,
    0., 1.);
  auto w = cml::transform_vector_2D(M,v);
  CATCH_CHECK(w[0] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(w[1] == Approx(3.).epsilon(1e-12));
}

CATCH_TEST_CASE("transform 2D, point1")
{
  auto p = cml::vector2d(2., 2.);
  auto M = cml::matrix33d(
    1., 0., 1.,
    0., 1., 0.,
    0., 0., 2.
    );
  auto q = cml::transform_point_2D(M,p);
  CATCH_CHECK(q[0] == Approx(3.).epsilon(1e-12));
  CATCH_CHECK(q[1] == Approx(2.).epsilon(1e-12));
}

CATCH_TEST_CASE("transform 2D, point2")
{
  auto p = cml::vector2d(2., 2.);
  auto M = cml::matrix33d_r(
    1., 0., 0.,
    0., 1., 0.,
    1., 0., 2.
    );
  auto q = cml::transform_point_2D(M,p);
  CATCH_CHECK(q[0] == Approx(3.).epsilon(1e-12));
  CATCH_CHECK(q[1] == Approx(2.).epsilon(1e-12));
}




CATCH_TEST_CASE("transform 3D, vector1")
{
  auto v = cml::vector3d(1., 1., 1.);
  auto M = cml::matrix33d(
    1., 0., 2.,
    0., 1., 0.,
    0., 0., 1.);
  auto w = cml::transform_vector(M,v);
  CATCH_CHECK(w[0] == Approx(3.).epsilon(1e-12));
  CATCH_CHECK(w[1] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(w[2] == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("transform 3D, vector2")
{
  auto v = cml::vector3d(1., 1., 1.);
  auto M = cml::matrix33d_r(
    1., 0., 2.,
    0., 1., 0.,
    0., 0., 1.);
  auto w = cml::transform_vector(M,v);
  CATCH_CHECK(w[0] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(w[1] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(w[2] == Approx(3.).epsilon(1e-12));
}

CATCH_TEST_CASE("transform 3D, point1")
{
  auto p = cml::vector3d(2., 2., 2.);
  auto M = cml::matrix44d(
    1., 0., 0., 1.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., 0., 0., 2.
    );
  auto q = cml::transform_point(M,p);
  CATCH_CHECK(q[0] == Approx(3.).epsilon(1e-12));
  CATCH_CHECK(q[1] == Approx(2.).epsilon(1e-12));
  CATCH_CHECK(q[2] == Approx(2.).epsilon(1e-12));
}

CATCH_TEST_CASE("transform 3D, point2")
{
  auto p = cml::vector3d(2., 2., 2.);
  auto M = cml::matrix44d_r(
    1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    1., 0., 0., 2.
    );
  auto q = cml::transform_point(M,p);
  CATCH_CHECK(q[0] == Approx(3.).epsilon(1e-12));
  CATCH_CHECK(q[1] == Approx(2.).epsilon(1e-12));
  CATCH_CHECK(q[2] == Approx(2.).epsilon(1e-12));
}

CATCH_TEST_CASE("transform 3D, hvector1")
{
  auto v = cml::vector4d(1., 1., 1., 0.);
  auto M = cml::matrix44d(
    1., 2., 3., 4.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., 0., 0., 1.
    );
  auto w = cml::transform_vector_4D(M,v);
  CATCH_CHECK(w[0] == Approx(6.).epsilon(1e-12));
  CATCH_CHECK(w[1] == 1.);
  CATCH_CHECK(w[2] == 1.);
  CATCH_CHECK(w[3] == 0.);
}

CATCH_TEST_CASE("transform 3D, hvector2")
{
  auto v = cml::vector4d(1., 1., 1., 0.);
  auto M = cml::matrix44d_r(
    1., 2., 3., 4.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., 0., 0., 1.
    );
  auto w = cml::transform_vector_4D(M,v);
  CATCH_CHECK(w[0] == 1.);
  CATCH_CHECK(w[1] == Approx(3.).epsilon(1e-12));
  CATCH_CHECK(w[2] == Approx(4.).epsilon(1e-12));
  CATCH_CHECK(w[3] == Approx(4.).epsilon(1e-12));
}

CATCH_TEST_CASE("transform 3D, hpoint1")
{
  auto p = cml::vector3d(2., 2., 2.);
  auto M = cml::matrix44d(
    1., 0., 0., 1.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., 0., 0., 2.
    );
  auto q = cml::transform_point_4D(M,p);
  CATCH_CHECK(q[0] == Approx(1.5).epsilon(1e-12));
  CATCH_CHECK(q[1] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(q[2] == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("transform 3D, hpoint2")
{
  auto p = cml::vector3d(2., 2., 2.);
  auto M = cml::matrix44d_r(
    1., 0., 0., 1.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., 0., 0., 2.
    );
  auto q = cml::transform_point_4D(M,p);
  CATCH_CHECK(q[0] == Approx(.5).epsilon(1e-12));
  CATCH_CHECK(q[1] == Approx(.5).epsilon(1e-12));
  CATCH_CHECK(q[2] == Approx(.5).epsilon(1e-12));
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
