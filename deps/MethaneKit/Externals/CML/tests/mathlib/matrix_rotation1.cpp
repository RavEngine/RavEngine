/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/mathlib/matrix/rotation.h>

#include <cml/vector.h>
#include <cml/matrix.h>
#include <cml/quaternion.h>
#include <cml/mathlib/quaternion/rotation.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("rotation 2D, rotation1")
{
  cml::matrix22d M;
  cml::matrix_rotation_2D(M, cml::rad(90.));
  auto v = M*cml::vector2d(0., 1.);
  CATCH_CHECK(v[0] == Approx(-1.).epsilon(1e-12));
  CATCH_CHECK(0 == Approx(v[1]).epsilon(0).margin(1e-7));
}

CATCH_TEST_CASE("rotation 2D, align1")
{
  cml::matrix22d M;
  cml::matrix_rotation_align_2D(M, cml::vector2d(0.,1.));

  auto v = M*cml::vector2d(0., 1.);
  CATCH_CHECK(v[0] == Approx(-1.).epsilon(1e-12));
  CATCH_CHECK(0 == Approx(v[1]).epsilon(0).margin(1e-7));
}




CATCH_TEST_CASE("rotation 3D, world_axis1")
{
  cml::matrix33d Mx; cml::matrix_rotation_world_x(Mx, cml::rad(90.));
  auto vx = Mx*cml::vector3d(0., 1., 0.);	// 0,0,1
  CATCH_CHECK(0 == Approx(vx[0]).epsilon(0).margin(1e-7));
  CATCH_CHECK(0 == Approx(vx[1]).epsilon(0).margin(1e-7));
  CATCH_CHECK(vx[2] == Approx(1.).epsilon(1e-12));

  cml::matrix33d My; cml::matrix_rotation_world_y(My, cml::rad(90.));
  auto vy = My*cml::vector3d(-1., 0., 0.);	// 0,0,1
  CATCH_CHECK(0 == Approx(vy[0]).epsilon(0).margin(1e-7));
  CATCH_CHECK(0 == Approx(vy[1]).epsilon(0).margin(1e-7));
  CATCH_CHECK(vy[2] == Approx(1.).epsilon(1e-12));

  cml::matrix33d Mz; cml::matrix_rotation_world_z(Mz, cml::rad(90.));
  auto vz = Mz*cml::vector3d(1., 0., 0.);	// 0,1,0
  CATCH_CHECK(0 == Approx(vz[0]).epsilon(0).margin(1e-7));
  CATCH_CHECK(vz[1] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(0 == Approx(vz[2]).epsilon(0).margin(1e-7));
}

CATCH_TEST_CASE("rotation 3D, rotation1")
{
  cml::matrix33d M;
  cml::matrix_rotation_axis_angle(
    M, cml::vector3d(1., 0., 0.), cml::rad(90.));
  auto v = M*cml::vector3d(0., 1., 0.);	// 0,0,1
  CATCH_CHECK(0 == Approx(v[0]).epsilon(0).margin(1e-7));
  CATCH_CHECK(0 == Approx(v[1]).epsilon(0).margin(1e-7));
  CATCH_CHECK(v[2] == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("rotation 3D, int_rotation1")
{
  cml::matrix33i M;
  cml::matrix_rotation_axis_angle(
    M, cml::vector3d(1., 0., 0.), cml::rad(90.));

  CATCH_CHECK(M(0,0) ==  1);
  CATCH_CHECK(M(1,2) == -1);
  CATCH_CHECK(M(2,1) ==  1);
}

CATCH_TEST_CASE("rotation 3D, euler1")
{
  cml::matrix33d M;
  cml::matrix_rotation_euler(
    M, cml::rad(90.), 0., 0., cml::euler_order_xyz);

  auto v = M*cml::vector3d(0., 1., 0.);	// 0,0,1
  CATCH_CHECK(0 == Approx(v[0]).epsilon(0).margin(1e-7));
  CATCH_CHECK(0 == Approx(v[1]).epsilon(0).margin(1e-7));
  CATCH_CHECK(v[2] == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("rotation 3D, euler_derivaties1")
{
  cml::matrix33d M;
  cml::matrix_rotation_euler_derivatives(
    M, 0, cml::rad(90.), 0., 0., cml::euler_order_xyz);

  auto v = M*cml::vector3d(0., 1., 0.);	// 0,0,1
  CATCH_CHECK(0 == Approx(v[0]).epsilon(0).margin(1e-7));
  CATCH_CHECK(v[1] == Approx(-1.).epsilon(1e-12));
  CATCH_CHECK(0 == Approx(v[2]).epsilon(0).margin(1e-7));
}

CATCH_TEST_CASE("rotation 3D, euler2")
{
  cml::matrix33d M;
  cml::matrix_rotation_euler(
    M, cml::vector3d(cml::rad(90.), 0., 0.), cml::euler_order_xyz);

  auto v = M*cml::vector3d(0., 1., 0.);	// 0,0,1
  CATCH_CHECK(0 == Approx(v[0]).epsilon(0).margin(1e-7));
  CATCH_CHECK(0 == Approx(v[1]).epsilon(0).margin(1e-7));
  CATCH_CHECK(v[2] == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("rotation 3D, euler_derivaties2")
{
  cml::matrix33d M;
  cml::matrix_rotation_euler_derivatives(
    M, 0, cml::vector3d(cml::rad(90.), 0., 0.), cml::euler_order_xyz);

  auto v = M*cml::vector3d(0., 1., 0.);	// 0,0,1
  CATCH_CHECK(0 == Approx(v[0]).epsilon(0).margin(1e-7));
  CATCH_CHECK(v[1] == Approx(-1.).epsilon(1e-12));
  CATCH_CHECK(0 == Approx(v[2]).epsilon(0).margin(1e-7));
}

CATCH_TEST_CASE("rotation 3D, quaternion1")
{
  cml::quaterniond q; cml::quaternion_rotation_axis_angle(
    q, cml::vector3d(1., 0., 0.), cml::rad(90.));
  cml::matrix33d M; cml::matrix_rotation_quaternion(M, q);
  auto v = M*cml::vector3d(0., 1., 0.);	// 0,0,1
  CATCH_CHECK(0 == Approx(v[0]).epsilon(0).margin(1e-7));
  CATCH_CHECK(0 == Approx(v[1]).epsilon(0).margin(1e-7));
  CATCH_CHECK(v[2] == Approx(1.).epsilon(1e-12));
}

CATCH_TEST_CASE("rotation 3D, align_ref1")
{
  cml::matrix33d M; cml::matrix_rotation_align(
    M, cml::vector3d(0., 0., 1.), cml::vector3d(1., 0., 0.));
  auto v = M*cml::vector3d(0., 1., 0.);	// 0,0,1
  CATCH_CHECK(v[0] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(0 == Approx(v[1]).epsilon(0).margin(1e-7));
  CATCH_CHECK(0 == Approx(v[2]).epsilon(0).margin(1e-7));
}

CATCH_TEST_CASE("rotation 3D, aim_at_ref1")
{
  cml::matrix33d M; cml::matrix_rotation_aim_at(M,
    cml::vector3d(0.,0.,0.), cml::vector3d(0., 0., 1.),
    cml::vector3d(1., 0., 0.));
  auto v = M*cml::vector3d(0., 1., 0.);	// 0,0,1
  CATCH_CHECK(v[0] == Approx(1.).epsilon(1e-12));
  CATCH_CHECK(0 == Approx(v[1]).epsilon(0).margin(1e-7));
  CATCH_CHECK(0 == Approx(v[2]).epsilon(0).margin(1e-7));
}

CATCH_TEST_CASE("rotation 3D, to_axis_angle1")
{
  cml::matrix33d M;
  cml::matrix_rotation_axis_angle(
    M, cml::vector3d(1., 2., 3.).normalize(), cml::rad(23.));

  cml::vector3d axis;
  double angle;
  cml::matrix_to_axis_angle(M, axis, angle);

  CATCH_CHECK(axis[0] == Approx(0.2672612419124244).epsilon(1e-12));
  CATCH_CHECK(axis[1] == Approx(0.53452248382484879).epsilon(1e-12));
  CATCH_CHECK(axis[2] == Approx(0.80178372573727308).epsilon(1e-12));
}

CATCH_TEST_CASE("rotation 3D, to_axis_angle_tuple1")
{
  cml::matrix33d M;
  cml::matrix_rotation_axis_angle(
    M, cml::vector3d(1., 2., 3.).normalize(), cml::rad(23.));

  cml::vector3d axis;
  double angle;
  std::tie(axis,angle) = cml::matrix_to_axis_angle(M);

  CATCH_CHECK(axis[0] == Approx(0.2672612419124244).epsilon(1e-12));
  CATCH_CHECK(axis[1] == Approx(0.53452248382484879).epsilon(1e-12));
  CATCH_CHECK(axis[2] == Approx(0.80178372573727308).epsilon(1e-12));
}

CATCH_TEST_CASE("rotation 3D, to_euler1")
{
  cml::matrix33d M;
  cml::matrix_rotation_euler(
    M, cml::rad(22.), cml::rad(10.), cml::rad(89.9), cml::euler_order_xyz);

  cml::vector3d v;
  cml::matrix_to_euler(M, v[0], v[1], v[2], cml::euler_order_xyz);

  CATCH_CHECK(v[0] == Approx(cml::rad(22.)).epsilon(1e-12));
  CATCH_CHECK(v[1] == Approx(cml::rad(10.)).epsilon(1e-12));
  CATCH_CHECK(v[2] == Approx(cml::rad(89.9)).epsilon(1e-12));
}

CATCH_TEST_CASE("rotation 3D, to_euler2")
{
  cml::matrix33d M;
  cml::matrix_rotation_euler(
    M, cml::rad(22.), cml::rad(10.), cml::rad(89.9), cml::euler_order_xyx);

  cml::vector3d v;
  cml::matrix_to_euler(M, v[0], v[1], v[2], cml::euler_order_xyx);

  CATCH_CHECK(v[0] == Approx(cml::rad(22.)).epsilon(1e-12));
  CATCH_CHECK(v[1] == Approx(cml::rad(10.)).epsilon(1e-12));
  CATCH_CHECK(v[2] == Approx(cml::rad(89.9)).epsilon(1e-12));
}

CATCH_TEST_CASE("rotation 3D, to_euler_vector1")
{
  cml::matrix33d M;
  cml::matrix_rotation_euler(
    M, cml::rad(22.), cml::rad(10.), cml::rad(89.9), cml::euler_order_xyz);

  auto v = cml::matrix_to_euler(M, cml::euler_order_xyz);
  CATCH_CHECK(v[0] == Approx(cml::rad(22.)).epsilon(1e-12));
  CATCH_CHECK(v[1] == Approx(cml::rad(10.)).epsilon(1e-12));
  CATCH_CHECK(v[2] == Approx(cml::rad(89.9)).epsilon(1e-12));
}

CATCH_TEST_CASE("rotation 3D, to_euler_vector2")
{
  cml::matrix33d M;
  cml::matrix_rotation_euler(
    M, cml::rad(22.), cml::rad(10.), cml::rad(89.9), cml::euler_order_xyz);

  auto v = cml::matrix_to_euler<cml::vectord>(M, cml::euler_order_xyz);
  CATCH_CHECK(v[0] == Approx(cml::rad(22.)).epsilon(1e-12));
  CATCH_CHECK(v[1] == Approx(cml::rad(10.)).epsilon(1e-12));
  CATCH_CHECK(v[2] == Approx(cml::rad(89.9)).epsilon(1e-12));
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
