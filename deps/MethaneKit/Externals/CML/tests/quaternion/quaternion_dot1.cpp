/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/quaternion/dot.h>

#include <cml/quaternion/fixed.h>
#include <cml/quaternion/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("dot1")
{
  cml::quaterniond v1 = { 1., 1., 1., 1. };
  cml::quaterniond v2 = { 2., 2., 2., 2. };
  double dp = cml::dot(v1,v2);
  CATCH_CHECK(dp == 8.);
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
