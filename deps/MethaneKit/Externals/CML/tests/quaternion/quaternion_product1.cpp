/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <iostream>

// Make sure the main header compiles cleanly:
#include <cml/quaternion/product.h>

#include <cml/quaternion/fixed.h>
#include <cml/quaternion/types.h>
#include <cml/util/vector_print.h>
#include <cml/util/quaternion_print.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("fixed, ip1")
{
  cml::quaterniond_ip q = { 1., 2., 3., 4. };
  cml::quaterniond_ip r = { 4., 3., 2., 1. };
  auto w = q*r;
  CATCH_CHECK(w[0] == 12.);
  CATCH_CHECK(w[1] == 24.);
  CATCH_CHECK(w[2] == 6.);
  CATCH_CHECK(w[3] == -12.);
}

CATCH_TEST_CASE("fixed, rp1")
{
  cml::quaterniond_rp q = { 1., 2., 3., 4. };
  cml::quaterniond_rp r = { 4., 3., 2., 1. };
  auto w = q*r;
  CATCH_CHECK(w[0] == -12.);
  CATCH_CHECK(w[1] == 6.);
  CATCH_CHECK(w[2] == 24.);
  CATCH_CHECK(w[3] == 12.);
}

CATCH_TEST_CASE("fixed, in1")
{
  cml::quaterniond_in q = { 1., 2., 3., 4. };
  cml::quaterniond_in r = { 4., 3., 2., 1. };
  auto w = q*r;
  CATCH_CHECK(w[0] == 22.);
  CATCH_CHECK(w[1] == 4.);
  CATCH_CHECK(w[2] == 16.);
  CATCH_CHECK(w[3] == -12.);
}

CATCH_TEST_CASE("fixed, rn1")
{
  cml::quaterniond_rn q = { 1., 2., 3., 4. };
  cml::quaterniond_rn r = { 4., 3., 2., 1. };
  auto w = q*r;
  CATCH_CHECK(w[0] == -12.);
  CATCH_CHECK(w[1] == 16.);
  CATCH_CHECK(w[2] == 4.);
  CATCH_CHECK(w[3] == 22.);
}

CATCH_TEST_CASE("fixed, ip_assign1")
{
  cml::quaterniond_ip q = { 1., 2., 3., 4. };
  cml::quaterniond_ip r = { 4., 3., 2., 1. };
  q *= r;
  CATCH_CHECK(q[0] == 12.);
  CATCH_CHECK(q[1] == 24.);
  CATCH_CHECK(q[2] == 6.);
  CATCH_CHECK(q[3] == -12.);
}

CATCH_TEST_CASE("fixed, rp_assign1")
{
  cml::quaterniond_rp q = { 1., 2., 3., 4. };
  cml::quaterniond_rp r = { 4., 3., 2., 1. };
  q *= r;
  CATCH_CHECK(q[0] == -12.);
  CATCH_CHECK(q[1] == 6.);
  CATCH_CHECK(q[2] == 24.);
  CATCH_CHECK(q[3] == 12.);
}

CATCH_TEST_CASE("fixed, in_assign1")
{
  cml::quaterniond_in q = { 1., 2., 3., 4. };
  cml::quaterniond_in r = { 4., 3., 2., 1. };
  q *= r;
  CATCH_CHECK(q[0] == 22.);
  CATCH_CHECK(q[1] == 4.);
  CATCH_CHECK(q[2] == 16.);
  CATCH_CHECK(q[3] == -12.);
}

CATCH_TEST_CASE("fixed, rn_assign1")
{
  cml::quaterniond_rn q = { 1., 2., 3., 4. };
  cml::quaterniond_rn r = { 4., 3., 2., 1. };
  q *= r;
  CATCH_CHECK(q[0] == -12.);
  CATCH_CHECK(q[1] == 16.);
  CATCH_CHECK(q[2] == 4.);
  CATCH_CHECK(q[3] == 22.);
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
