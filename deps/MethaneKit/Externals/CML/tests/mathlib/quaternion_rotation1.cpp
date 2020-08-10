/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/mathlib/quaternion/rotation.h>

#include <cml/vector.h>
#include <cml/matrix.h>
#include <cml/quaternion.h>
#include <cml/mathlib/matrix/rotation.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("world_axis1")
{
  cml::quaterniond qx; cml::quaternion_rotation_world_x(qx, M_PI/3.);
  CATCH_CHECK(qx.real() == Approx(0.86602540378443871).epsilon(1e-12));
  CATCH_CHECK(qx.imaginary()[0] == Approx(0.49999999999999994).epsilon(1e-12));
  CATCH_CHECK(qx.imaginary()[1] == 0.);
  CATCH_CHECK(qx.imaginary()[2] == 0.);

  cml::quaterniond qy; cml::quaternion_rotation_world_y(qy, M_PI/2.);
  CATCH_CHECK(qy.real() == Approx(0.70710678118654757).epsilon(1e-12));
  CATCH_CHECK(qy.imaginary()[0] == 0.);
  CATCH_CHECK(qy.imaginary()[1] == Approx(0.70710678118654757).epsilon(1e-12));
  CATCH_CHECK(qy.imaginary()[2] == 0.);

  cml::quaterniond qz; cml::quaternion_rotation_world_z(qz, M_PI);
  CATCH_CHECK(0 == Approx(qz.real()).epsilon(0).margin(2e-16));
  CATCH_CHECK(qz.imaginary()[0] == 0.);
  CATCH_CHECK(qz.imaginary()[1] == 0.);
  CATCH_CHECK(qz.imaginary()[2] == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("axis_angle1")
{
  cml::quaterniond q; cml::quaternion_rotation_axis_angle(
    q, cml::vector3d(1.,1.,1.).normalize(), M_PI/3.);
  CATCH_CHECK(q.real() == Approx(0.86602540378443871).epsilon(1e-12));
  CATCH_CHECK(q.imaginary()[0] == Approx(0.28867513459481287).epsilon(1e-12));
  CATCH_CHECK(q.imaginary()[1] == Approx(0.28867513459481287).epsilon(1e-12));
  CATCH_CHECK(q.imaginary()[2] == Approx(0.28867513459481287).epsilon(1e-12));
}

CATCH_TEST_CASE("matrix1")
{
  cml::matrix33d M; cml::matrix_rotation_axis_angle(
    M, cml::vector3d(1.,1.,1.).normalize(), M_PI/3.);
  cml::quaterniond q; cml::quaternion_rotation_matrix(q, M);

  CATCH_CHECK(q.real() == Approx(0.86602540378443871).epsilon(1e-12));
  CATCH_CHECK(q.imaginary()[0] == Approx(0.28867513459481287).epsilon(1e-12));
  CATCH_CHECK(q.imaginary()[1] == Approx(0.28867513459481287).epsilon(1e-12));
  CATCH_CHECK(q.imaginary()[2] == Approx(0.28867513459481287).epsilon(1e-12));
}

CATCH_TEST_CASE("align_ref1")
{
  cml::quaterniond q; cml::quaternion_rotation_align(
    q, cml::vector3d(0., 0., 1.), cml::vector3d(1., 0., 0.));
  cml::matrix33d M; cml::matrix_rotation_quaternion(M, q);
  auto v = M*cml::vector3d(0., 1., 0.);	// 0,0,1
  CATCH_CHECK(v[0] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(0 == Approx(v[1]).epsilon(0).margin(1e-7));
  CATCH_CHECK(0 == Approx(v[2]).epsilon(0).margin(1e-7));
}

CATCH_TEST_CASE("aim_at_ref1")
{
  cml::quaterniond q; cml::quaternion_rotation_aim_at(
    q, cml::vector3d(0.,0.,0.), cml::vector3d(0., 0., 1.),
    cml::vector3d(1., 0., 0.));
  cml::matrix33d M; cml::matrix_rotation_quaternion(M, q);
  auto v = M*cml::vector3d(0., 1., 0.);	// 0,0,1
  CATCH_CHECK(v[0] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(0 == Approx(v[1]).epsilon(0).margin(1e-7));
  CATCH_CHECK(0 == Approx(v[2]).epsilon(0).margin(1e-7));
}

CATCH_TEST_CASE("euler1")
{
  cml::quaterniond q;
  cml::quaternion_rotation_euler(
    q, cml::rad(90.), 0., 0., cml::euler_order_xyz);
  cml::matrix33d M; cml::matrix_rotation_quaternion(M, q);
  auto v = M*cml::vector3d(0., 1., 0.);	// 0,0,1
  CATCH_CHECK(0 == Approx(v[0]).epsilon(0).margin(1e-7));
  CATCH_CHECK(0 == Approx(v[1]).epsilon(0).margin(1e-7));
  CATCH_CHECK(v[2] == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("euler2")
{
  cml::quaterniond q;
  cml::quaternion_rotation_euler(
    q, cml::vector3d(cml::rad(90.), 0., 0.), cml::euler_order_xyz);
  cml::matrix33d M; cml::matrix_rotation_quaternion(M, q);
  auto v = M*cml::vector3d(0., 1., 0.);	// 0,0,1
  CATCH_CHECK(0 == Approx(v[0]).epsilon(0).margin(1e-7));
  CATCH_CHECK(0 == Approx(v[1]).epsilon(0).margin(1e-7));
  CATCH_CHECK(v[2] == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("to_axis_angle1")
{
  cml::quaterniond q;
  cml::quaternion_rotation_axis_angle(
    q, cml::vector3d(1., 2., 3.).normalize(), cml::rad(23.));

  cml::vector3d axis;
  double angle;
  cml::quaternion_to_axis_angle(q, axis, angle);

  CATCH_CHECK(axis[0] == Approx(0.2672612419124244).epsilon(1e-12));
  CATCH_CHECK(axis[1] == Approx(0.53452248382484879).epsilon(1e-12));
  CATCH_CHECK(axis[2] == Approx(0.80178372573727308).epsilon(1e-12));
}

CATCH_TEST_CASE("to_axis_angle_tuple1")
{
  cml::quaterniond q;
  cml::quaternion_rotation_axis_angle(
    q, cml::vector3d(1., 2., 3.).normalize(), cml::rad(23.));

  cml::vector3d axis;
  double angle;
  std::tie(axis,angle) = cml::quaternion_to_axis_angle(q);

  CATCH_CHECK(axis[0] == Approx(0.2672612419124244).epsilon(1e-12));
  CATCH_CHECK(axis[1] == Approx(0.53452248382484879).epsilon(1e-12));
  CATCH_CHECK(axis[2] == Approx(0.80178372573727308).epsilon(1e-12));
}

CATCH_TEST_CASE("to_euler1")
{
  cml::quaterniond q;
  cml::quaternion_rotation_euler(
    q, cml::rad(22.), cml::rad(10.), cml::rad(89.9), cml::euler_order_xyz);

  cml::vector3d v;
  cml::quaternion_to_euler(q, v[0], v[1], v[2], cml::euler_order_xyz);

  CATCH_CHECK(v[0] == Approx(cml::rad(22.)).epsilon(1e-12));
  CATCH_CHECK(v[1] == Approx(cml::rad(10.)).epsilon(1e-12));
  CATCH_CHECK(v[2] == Approx(cml::rad(89.9)).epsilon(1e-12));
}

CATCH_TEST_CASE("to_euler2")
{
  cml::quaterniond q;
  cml::quaternion_rotation_euler(
    q, cml::rad(22.), cml::rad(10.), cml::rad(89.9), cml::euler_order_xyx);

  cml::vector3d v;
  cml::quaternion_to_euler(q, v[0], v[1], v[2], cml::euler_order_xyx);

  CATCH_CHECK(v[0] == Approx(cml::rad(22.)).epsilon(1e-12));
  CATCH_CHECK(v[1] == Approx(cml::rad(10.)).epsilon(1e-12));
  CATCH_CHECK(v[2] == Approx(cml::rad(89.9)).epsilon(1e-12));
}

CATCH_TEST_CASE("to_euler_vector1")
{
  cml::quaterniond q;
  cml::quaternion_rotation_euler(
    q, cml::rad(22.), cml::rad(10.), cml::rad(89.9), cml::euler_order_xyz);

  auto v = cml::quaternion_to_euler(q, cml::euler_order_xyz);
  CATCH_CHECK(v[0] == Approx(cml::rad(22.)).epsilon(1e-12));
  CATCH_CHECK(v[1] == Approx(cml::rad(10.)).epsilon(1e-12));
  CATCH_CHECK(v[2] == Approx(cml::rad(89.9)).epsilon(1e-12));
}

CATCH_TEST_CASE("to_euler_vector2")
{
  cml::quaterniond q;
  cml::quaternion_rotation_euler(
    q, cml::rad(22.), cml::rad(10.), cml::rad(89.9), cml::euler_order_xyz);

  auto v = cml::quaternion_to_euler<cml::vectord>(q, cml::euler_order_xyz);
  CATCH_CHECK(v[0] == Approx(cml::rad(22.)).epsilon(1e-12));
  CATCH_CHECK(v[1] == Approx(cml::rad(10.)).epsilon(1e-12));
  CATCH_CHECK(v[2] == Approx(cml::rad(89.9)).epsilon(1e-12));
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
