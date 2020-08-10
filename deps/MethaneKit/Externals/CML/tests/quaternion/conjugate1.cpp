/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/quaternion/conjugate_node.h>

#include <cml/quaternion/fixed.h>
#include <cml/quaternion/conjugate.h>
#include <cml/quaternion/types.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("fixed, readable1")
{
  const cml::quaterniond_ip q(1., 2., 3., 4.);
  auto xpr = q.conjugate();
  CATCH_CHECK(xpr[0] == - 1.);
  CATCH_CHECK(xpr[1] == - 2.);
  CATCH_CHECK(xpr[2] == - 3.);
  CATCH_CHECK(xpr[3] ==   4.);
}

CATCH_TEST_CASE("fixed, readable2")
{
  const cml::quaterniond_rp q(1., 2., 3., 4.);
  auto xpr = q.conjugate();
  CATCH_CHECK(xpr[0] ==   1.);
  CATCH_CHECK(xpr[1] == - 2.);
  CATCH_CHECK(xpr[2] == - 3.);
  CATCH_CHECK(xpr[3] == - 4.);
}

CATCH_TEST_CASE("fixed, writable1")
{
  cml::quaterniond_ip q = { 1., 2., 3., 4. };
  q.conjugate();
  CATCH_CHECK(q[0] == - 1.);
  CATCH_CHECK(q[1] == - 2.);
  CATCH_CHECK(q[2] == - 3.);
  CATCH_CHECK(q[3] ==   4.);
}

CATCH_TEST_CASE("fixed, writable2")
{
  cml::quaterniond_rp q = { 1., 2., 3., 4. };
  q.conjugate();
  CATCH_CHECK(q[0] ==   1.);
  CATCH_CHECK(q[1] == - 2.);
  CATCH_CHECK(q[2] == - 3.);
  CATCH_CHECK(q[3] == - 4.);
}

CATCH_TEST_CASE("fixed, function1")
{
  cml::quaterniond_ip q = { 1., 2., 3., 4. };
  auto xpr = cml::conjugate(q);
  CATCH_CHECK(xpr[0] == - 1.);
  CATCH_CHECK(xpr[1] == - 2.);
  CATCH_CHECK(xpr[2] == - 3.);
  CATCH_CHECK(xpr[3] ==   4.);
}

CATCH_TEST_CASE("fixed, function2")
{
  cml::quaterniond_rp q = { 1., 2., 3., 4. };
  auto xpr = cml::conjugate(q);
  CATCH_CHECK(xpr[0] ==   1.);
  CATCH_CHECK(xpr[1] == - 2.);
  CATCH_CHECK(xpr[2] == - 3.);
  CATCH_CHECK(xpr[3] == - 4.);
}

CATCH_TEST_CASE("fixed, temporary1")
{
  auto xpr = cml::conjugate(cml::quaterniond_ip(1., 2., 3., 4.));
  CATCH_CHECK(xpr[0] == - 1.);
  CATCH_CHECK(xpr[1] == - 2.);
  CATCH_CHECK(xpr[2] == - 3.);
  CATCH_CHECK(xpr[3] ==   4.);
}

CATCH_TEST_CASE("fixed, conjugate2")
{
  auto xpr = cml::conjugate(cml::quaterniond_rp(1., 2., 3., 4.));
  CATCH_CHECK(xpr[0] ==   1.);
  CATCH_CHECK(xpr[1] == - 2.);
  CATCH_CHECK(xpr[2] == - 3.);
  CATCH_CHECK(xpr[3] == - 4.);
}



// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
