/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/quaternion/functions.h>

#include <cml/quaternion.h>
#include <cml/util/vector_print.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("fixed, real1")
{
  cml::quaterniond_ip q = { 1., 2., 3., 4. };
  CATCH_REQUIRE(q.size() == 4);
  CATCH_CHECK(cml::real(q) == 4.);
}

CATCH_TEST_CASE("fixed, real2")
{
  cml::quaterniond_rp q = { 1., 2., 3., 4. };
  CATCH_REQUIRE(q.size() == 4);
  CATCH_CHECK(cml::real(q) == 1.);
}

CATCH_TEST_CASE("fixed, length_squared1")
{
  cml::quaterniond q = { 1., 2., 3., 4. };
  CATCH_REQUIRE(q.size() == 4);
  CATCH_CHECK(cml::length_squared(q) == Approx(30.).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, length1")
{
  cml::quaterniond q = { 1., 2., 3., 4. };
  CATCH_REQUIRE(q.size() == 4);
  CATCH_CHECK(cml::length(q) == Approx(std::sqrt(30.)).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, norm1")
{
  cml::quaterniond q = { 1., 2., 3., 4. };
  CATCH_REQUIRE(q.size() == 4);
  CATCH_CHECK(cml::norm(q) == Approx(30.).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, normalize1")
{
  cml::quaterniond q = { 1., 2., 3., 4. };
  q.normalize();
  double l2 = q.length_squared();
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, normalize2")
{
  cml::quaterniond q = { 1., 2., 3., 4. };
  double l2 = cml::normalize(q).length_squared();
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, identity1")
{
  cml::quaterniond q = { 1., 2., 3., 4. };
  double l2 = cml::identity(q).length_squared();
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, zero1")
{
  cml::quaterniond q;
  q.zero();
  CATCH_CHECK(q[0] == 0.);
  CATCH_CHECK(q[1] == 0.);
  CATCH_CHECK(q[2] == 0.);
  CATCH_CHECK(q[3] == 0.);
  CATCH_CHECK(q.length() == 0.);
}

CATCH_TEST_CASE("fixed, log1")
{
  cml::quaterniond_ip q = { 1., 2., 3., 4. };
  CATCH_REQUIRE(q.size() == 4);
  auto lnq = cml::log(q);
  CATCH_CHECK(lnq[0] == Approx(0.200991168205).epsilon(1e-12));
  CATCH_CHECK(lnq[1] == Approx(0.401982336411).epsilon(1e-12));
  CATCH_CHECK(lnq[2] == Approx(0.602973504616).epsilon(1e-12));
  CATCH_CHECK(lnq[3] == Approx(1.700598690831).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, log2")
{
  cml::quaterniond_rp q = { 1., 2., 3., 4. };
  CATCH_REQUIRE(q.size() == 4);
  auto lnq = cml::log(q);
  CATCH_CHECK(lnq[0] == Approx(1.700598690831).epsilon(1e-12));
  CATCH_CHECK(lnq[1] == Approx(0.515190292664).epsilon(1e-12));
  CATCH_CHECK(lnq[2] == Approx(0.772785438996).epsilon(1e-12));
  CATCH_CHECK(lnq[3] == Approx(1.030380585328).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, log3")
{
  auto lnq = cml::quaterniond_ip(1., 2., 3., 4.).log();
  CATCH_CHECK(lnq[0] == Approx(0.200991168205).epsilon(1e-12));
  CATCH_CHECK(lnq[1] == Approx(0.401982336411).epsilon(1e-12));
  CATCH_CHECK(lnq[2] == Approx(0.602973504616).epsilon(1e-12));
  CATCH_CHECK(lnq[3] == Approx(1.700598690831).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, log4")
{
  auto lnq = cml::quaterniond_rp(1., 2., 3., 4.).log();
  CATCH_CHECK(lnq[0] == Approx(1.700598690831).epsilon(1e-12));
  CATCH_CHECK(lnq[1] == Approx(0.515190292664).epsilon(1e-12));
  CATCH_CHECK(lnq[2] == Approx(0.772785438996).epsilon(1e-12));
  CATCH_CHECK(lnq[3] == Approx(1.030380585328).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, exp1")
{
  cml::quaterniond_ip q = { 1., 2., 3., 4. };
  CATCH_REQUIRE(q.size() == 4);
  auto lnq = cml::exp(q);
  CATCH_CHECK(lnq[0] == Approx(-8.24002526676).epsilon(1e-12));
  CATCH_CHECK(lnq[1] == Approx(-16.4800505335).epsilon(1e-12));
  CATCH_CHECK(lnq[2] == Approx(-24.72007580027).epsilon(1e-12));
  CATCH_CHECK(lnq[3] == Approx(-45.0598020134).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, exp2")
{
  cml::quaterniond_rp q = { 1., 2., 3., 4. };
  CATCH_REQUIRE(q.size() == 4);
  auto lnq = cml::exp(q);
  CATCH_CHECK(lnq[0] == Approx( 1.693922723683).epsilon(1e-12));
  CATCH_CHECK(lnq[1] == Approx(-0.78955962454).epsilon(1e-12));
  CATCH_CHECK(lnq[2] == Approx(-1.184339436812).epsilon(1e-12));
  CATCH_CHECK(lnq[3] == Approx(-1.579119249083).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, exp3")
{
  auto lnq = cml::quaterniond_ip(1., 2., 3., 4.).exp();
  CATCH_CHECK(lnq[0] == Approx(-8.24002526676).epsilon(1e-12));
  CATCH_CHECK(lnq[1] == Approx(-16.4800505335).epsilon(1e-12));
  CATCH_CHECK(lnq[2] == Approx(-24.72007580027).epsilon(1e-12));
  CATCH_CHECK(lnq[3] == Approx(-45.0598020134).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, exp4")
{
  auto lnq = cml::quaterniond_rp(1., 2., 3., 4.).exp();
  CATCH_CHECK(lnq[0] == Approx( 1.693922723683).epsilon(1e-12));
  CATCH_CHECK(lnq[1] == Approx(-0.78955962454).epsilon(1e-12));
  CATCH_CHECK(lnq[2] == Approx(-1.184339436812).epsilon(1e-12));
  CATCH_CHECK(lnq[3] == Approx(-1.579119249083).epsilon(1e-12));
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
