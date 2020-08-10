/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/quaternion/inverse_node.h>

#include <cml/quaternion/fixed.h>
#include <cml/quaternion/inverse.h>
#include <cml/quaternion/types.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("fixed, readable1")
{
  const cml::quaterniond_ip q(1., 2., 3., 4.);
  auto n = q.norm();
  auto xpr = q.inverse();
  CATCH_CHECK(xpr[0] == - 1./n);
  CATCH_CHECK(xpr[1] == - 2./n);
  CATCH_CHECK(xpr[2] == - 3./n);
  CATCH_CHECK(xpr[3] ==   4./n);
}

CATCH_TEST_CASE("fixed, readable2")
{
  const cml::quaterniond_rp q(1., 2., 3., 4.);
  auto n = q.norm();
  auto xpr = q.inverse();
  CATCH_CHECK(xpr[0] ==   1./n);
  CATCH_CHECK(xpr[1] == - 2./n);
  CATCH_CHECK(xpr[2] == - 3./n);
  CATCH_CHECK(xpr[3] == - 4./n);
}

CATCH_TEST_CASE("fixed, writable1")
{
  cml::quaterniond_ip q = { 1., 2., 3., 4. };
  auto n = q.norm();
  q.inverse();
  CATCH_CHECK(q[0] == - 1./n);
  CATCH_CHECK(q[1] == - 2./n);
  CATCH_CHECK(q[2] == - 3./n);
  CATCH_CHECK(q[3] ==   4./n);
}

CATCH_TEST_CASE("fixed, writable2")
{
  cml::quaterniond_rp q = { 1., 2., 3., 4. };
  auto n = q.norm();
  q.inverse();
  CATCH_CHECK(q[0] ==   1./n);
  CATCH_CHECK(q[1] == - 2./n);
  CATCH_CHECK(q[2] == - 3./n);
  CATCH_CHECK(q[3] == - 4./n);
}

CATCH_TEST_CASE("fixed, function1")
{
  cml::quaterniond_ip q = { 1., 2., 3., 4. };
  auto n = q.norm();
  auto xpr = cml::inverse(q);
  CATCH_CHECK(xpr[0] == - 1./n);
  CATCH_CHECK(xpr[1] == - 2./n);
  CATCH_CHECK(xpr[2] == - 3./n);
  CATCH_CHECK(xpr[3] ==   4./n);
}

CATCH_TEST_CASE("fixed, function2")
{
  cml::quaterniond_rp q = { 1., 2., 3., 4. };
  auto n = q.norm();
  auto xpr = cml::inverse(q);
  CATCH_CHECK(xpr[0] ==   1./n);
  CATCH_CHECK(xpr[1] == - 2./n);
  CATCH_CHECK(xpr[2] == - 3./n);
  CATCH_CHECK(xpr[3] == - 4./n);
}

CATCH_TEST_CASE("fixed, temporary1")
{
  auto xpr = cml::inverse(cml::quaterniond_ip(1., 2., 3., 4.));
  auto n = cml::quaterniond_ip(1., 2., 3., 4.).norm();
  CATCH_CHECK(xpr[0] == - 1./n);
  CATCH_CHECK(xpr[1] == - 2./n);
  CATCH_CHECK(xpr[2] == - 3./n);
  CATCH_CHECK(xpr[3] ==   4./n);
}

CATCH_TEST_CASE("fixed, inverse2")
{
  auto xpr = cml::inverse(cml::quaterniond_rp(1., 2., 3., 4.));
  auto n = cml::quaterniond_rp(1., 2., 3., 4.).norm();
  CATCH_CHECK(xpr[0] ==   1./n);
  CATCH_CHECK(xpr[1] == - 2./n);
  CATCH_CHECK(xpr[2] == - 3./n);
  CATCH_CHECK(xpr[3] == - 4./n);
}



// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
