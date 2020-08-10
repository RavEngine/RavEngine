/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/mathlib/quaternion/basis.h>

#include <cml/vector.h>
#include <cml/quaternion.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("ip1")
{
  auto q = cml::quaterniond_ip(1., 2., 3., 4.).normalize();
  auto qx = cml::quaternion_get_x_basis_vector(q);
  CATCH_CHECK(qx[0] == Approx(0.13333333333333353).epsilon(1e-12));
  CATCH_CHECK(qx[1] == Approx(0.93333333333333324).epsilon(1e-12));
  CATCH_CHECK(qx[2] == Approx(-0.3333333333333332).epsilon(1e-12));

  auto qy = cml::quaternion_get_y_basis_vector(q);
  CATCH_CHECK(qy[0] == Approx(-0.6666666666666666).epsilon(1e-12));
  CATCH_CHECK(qy[1] == Approx(0.33333333333333348).epsilon(1e-12));
  CATCH_CHECK(qy[2] == Approx(0.66666666666666652).epsilon(1e-12));

  auto qz = cml::quaternion_get_z_basis_vector(q);
  CATCH_CHECK(qz[0] == Approx(0.73333333333333317).epsilon(1e-12));
  CATCH_CHECK(qz[1] == Approx(0.13333333333333336).epsilon(1e-12));
  CATCH_CHECK(qz[2] == Approx(0.66666666666666674).epsilon(1e-12));
}

CATCH_TEST_CASE("rp1")
{
  auto q = cml::quaterniond_rp(4., 1., 2., 3.).normalize();
  auto qx = cml::quaternion_get_x_basis_vector(q);
  CATCH_CHECK(qx[0] == Approx(0.13333333333333353).epsilon(1e-12));
  CATCH_CHECK(qx[1] == Approx(0.93333333333333324).epsilon(1e-12));
  CATCH_CHECK(qx[2] == Approx(-0.3333333333333332).epsilon(1e-12));

  auto qy = cml::quaternion_get_y_basis_vector(q);
  CATCH_CHECK(qy[0] == Approx(-0.6666666666666666).epsilon(1e-12));
  CATCH_CHECK(qy[1] == Approx(0.33333333333333348).epsilon(1e-12));
  CATCH_CHECK(qy[2] == Approx(0.66666666666666652).epsilon(1e-12));

  auto qz = cml::quaternion_get_z_basis_vector(q);
  CATCH_CHECK(qz[0] == Approx(0.73333333333333317).epsilon(1e-12));
  CATCH_CHECK(qz[1] == Approx(0.13333333333333336).epsilon(1e-12));
  CATCH_CHECK(qz[2] == Approx(0.66666666666666674).epsilon(1e-12));
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
