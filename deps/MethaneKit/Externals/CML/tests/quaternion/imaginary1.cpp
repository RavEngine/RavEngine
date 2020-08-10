/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/quaternion/imaginary_node.h>

#include <cml/vector/fixed.h>
#include <cml/quaternion/fixed.h>
#include <cml/quaternion/imaginary.h>
#include <cml/quaternion/types.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("fixed, readable1")
{
  const cml::quaterniond_ip q(1., 2., 3., 4.);
  auto xpr = q.imaginary();
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 3);
  CATCH_CHECK(xpr[0] == 1.);
  CATCH_CHECK(xpr[1] == 2.);
  CATCH_CHECK(xpr[2] == 3.);
}

CATCH_TEST_CASE("fixed, readable2")
{
  const cml::quaterniond_rp q(1., 2., 3., 4.);
  auto xpr = q.imaginary();
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 3);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
  CATCH_CHECK(xpr[2] == 4.);
}

CATCH_TEST_CASE("fixed, function1")
{
  cml::quaterniond_ip q = { 1., 2., 3., 4. };
  auto xpr = cml::imaginary(q);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 3);
  CATCH_CHECK(xpr[0] == 1.);
  CATCH_CHECK(xpr[1] == 2.);
  CATCH_CHECK(xpr[2] == 3.);
}

CATCH_TEST_CASE("fixed, function2")
{
  cml::quaterniond_rp q = { 1., 2., 3., 4. };
  auto xpr = cml::imaginary(q);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 3);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
  CATCH_CHECK(xpr[2] == 4.);
}

CATCH_TEST_CASE("fixed, temporary1")
{
  auto xpr = cml::imaginary(cml::quaterniond_ip(1., 2., 3., 4.));
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 3);
  CATCH_CHECK(xpr[0] == 1.);
  CATCH_CHECK(xpr[1] == 2.);
  CATCH_CHECK(xpr[2] == 3.);
}

CATCH_TEST_CASE("fixed, temporary2")
{
  auto xpr = cml::imaginary(cml::quaterniond_rp(1., 2., 3., 4.));
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 3);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
  CATCH_CHECK(xpr[2] == 4.);
}



// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
