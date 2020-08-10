/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <type_traits>

// Make sure the main header compiles cleanly:
#include <cml/vector/functions.h>

#include <cml/vector.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("fixed, length_squared1")
{
  cml::vector3d v1 = { 1., 1., 1. };
  double l2 = v1.length_squared();
  CATCH_CHECK(l2 == 3.);
}

CATCH_TEST_CASE("fixed, length_squared2")
{
  cml::vector3d v1 = { 1., 1., 1. };
  double l2 = cml::length_squared(v1);
  CATCH_CHECK(l2 == 3.);
}

CATCH_TEST_CASE("fixed, length1")
{
  cml::vector3d v1 = { 1., 1., 1. };
  double l = v1.length();
  CATCH_CHECK(l == Approx(std::sqrt(3.)).epsilon(1e-4));
}

CATCH_TEST_CASE("fixed, length2")
{
  cml::vector3d v1 = { 1., 1., 1. };
  double l = cml::length(v1);
  CATCH_CHECK(l == Approx(std::sqrt(3.)).epsilon(1e-4));
}

CATCH_TEST_CASE("fixed, normalize1")
{
  cml::vector3d v1 = { 1., 1., 1. };
  v1.normalize();
  double l2 = v1.length_squared();
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, normalize2")
{
  cml::vector3d v1 = { 1., 1., 1. };
  double l2 = cml::normalize(v1).length_squared();
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, normalize3")
{
  CATCH_CHECK((std::is_same<decltype(
    cml::normalize(cml::vector3d())), cml::vector3d>::value));
  double l2 = cml::normalize(
    cml::vector3d(1., 1., 1.)).length_squared();
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, normalize4")
{
  cml::vector3d v1 = { 1., 1., 1. };
  cml::vector3d v2 = { 1., 1., 1. };
  cml::vector3d v3 = { 1., 1., 1. };
  auto xpr = cml::normalize(v1 + 2.*v2);
  CATCH_CHECK((std::is_same<decltype(xpr), cml::vector3d>::value));
  CATCH_CHECK(xpr.length_squared() == Approx(1.0).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, zero1")
{
  cml::vector3d v1 = { 1., 1., 1. };
  v1.zero();
  CATCH_CHECK(v1[0] == 0.);
  CATCH_CHECK(v1[1] == 0.);
  CATCH_CHECK(v1[2] == 0.);
  CATCH_CHECK(v1.length() == 0.);
}

CATCH_TEST_CASE("fixed, minimize1")
{
  cml::vector3d v1 = { 1., 1., 1. };
  cml::vector3d v2 = { 2., 0., 3. };
  v1.minimize(v2);
  CATCH_CHECK(v1[0] == 1.);
  CATCH_CHECK(v1[1] == 0.);
  CATCH_CHECK(v1[2] == 1.);
}

CATCH_TEST_CASE("fixed, maximize1")
{
  cml::vector3d v1 = { 1., 1., 1. };
  cml::vector3d v2 = { 2., 0., 3. };
  v1.maximize(v2);
  CATCH_CHECK(v1[0] == 2.);
  CATCH_CHECK(v1[1] == 1.);
  CATCH_CHECK(v1[2] == 3.);
}

CATCH_TEST_CASE("fixed, cardinal1")
{
  cml::vector3d v1;
  v1.cardinal(0);
  CATCH_CHECK(v1[0] == 1.);
  CATCH_CHECK(v1[1] == 0.);
  CATCH_CHECK(v1[2] == 0.);
}

CATCH_TEST_CASE("fixed, random1")
{
  cml::vector4d v1;
  v1.random(0.,1.);
  for(const auto& e : v1) {
    CATCH_CHECK(e >= 0.);
    CATCH_CHECK(e < 1.);
  }
}




CATCH_TEST_CASE("fixed external, length_squared1")
{
  double av1[] = { 1., 1., 1. };
  cml::external3d v1(av1);
  double l2 = v1.length_squared();
  CATCH_CHECK(l2 == 3.);
}

CATCH_TEST_CASE("fixed external, length1")
{
  double av1[] = { 1., 1., 1. };
  cml::external3d v1(av1);
  double l = v1.length();
  CATCH_CHECK(l == Approx(std::sqrt(3.)).epsilon(1e-4));
}

CATCH_TEST_CASE("fixed external, normalize1")
{
  double av1[] = { 1., 1., 1. };
  cml::external3d v1(av1);
  double l2 = v1.normalize().length_squared();
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed external, normalize2")
{
  double av1[] = { 1., 1., 1. };
  cml::external3d v1(av1);
  double l2 = cml::normalize(v1).length_squared();
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed external, zero1")
{
  double av1[] = { 1., 1., 1. };
  cml::external3d v1(av1);
  v1.zero();
  CATCH_CHECK(v1[0] == 0.);
  CATCH_CHECK(v1[1] == 0.);
  CATCH_CHECK(v1[2] == 0.);
  CATCH_CHECK(v1.length() == 0.);
}

CATCH_TEST_CASE("fixed external, minimize1")
{
  double av1[] = { 1., 1., 1. };
  double av2[] = { 2., 0., 3. };
  cml::external3d v1(av1);
  cml::external3d v2(av2);
  v1.minimize(v2);
  CATCH_CHECK(v1[0] == 1.);
  CATCH_CHECK(v1[1] == 0.);
  CATCH_CHECK(v1[2] == 1.);
}

CATCH_TEST_CASE("fixed external, maximize1")
{
  double av1[] = { 1., 1., 1. };
  double av2[] = { 2., 0., 3. };
  cml::external3d v1(av1);
  cml::external3d v2(av2);
  v1.maximize(v2);
  CATCH_CHECK(v1[0] == 2.);
  CATCH_CHECK(v1[1] == 1.);
  CATCH_CHECK(v1[2] == 3.);
}

CATCH_TEST_CASE("fixed external, cardinal1")
{
  double av1[3];
  cml::external3d v1(av1);
  v1.cardinal(0);
  CATCH_CHECK(v1[0] == 1.);
  CATCH_CHECK(v1[1] == 0.);
  CATCH_CHECK(v1[2] == 0.);
}

CATCH_TEST_CASE("fixed external, random1")
{
  double av1[4];
  cml::external3d v1(av1);
  v1.random(0.,1.);
  for(const auto& e : v1) {
    CATCH_CHECK(e >= 0.);
    CATCH_CHECK(e < 1.);
  }
}




CATCH_TEST_CASE("fixed const external, length_squared1")
{
  const double av1[] = { 1., 1., 1. };
  cml::external3cd v1(av1);
  double l2 = v1.length_squared();
  CATCH_CHECK(l2 == 3.);
}

CATCH_TEST_CASE("fixed const external, length1")
{
  const double av1[] = { 1., 1., 1. };
  cml::external3cd v1(av1);
  double l = v1.length();
  CATCH_CHECK(l == Approx(std::sqrt(3.)).epsilon(1e-4));
}

CATCH_TEST_CASE("fixed const external, normalize1")
{
  const double av1[] = { 1., 1., 1. };
  cml::external3cd v1(av1);
  double l2 = v1.normalize().length_squared();
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed const external, normalize2")
{
  const double av1[] = { 1., 1., 1. };
  cml::external3cd v1(av1);
  double l2 = cml::normalize(v1).length_squared();
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}




CATCH_TEST_CASE("dynamic, length_squared1")
{
  cml::vectord v1 = { 1., 1., 1. };
  double l2 = v1.length_squared();
  CATCH_CHECK(l2 == 3.);
}

CATCH_TEST_CASE("dynamic, length1")
{
  cml::vectord v1 = { 1., 1., 1. };
  double l = v1.length();
  CATCH_CHECK(l == Approx(std::sqrt(3.)).epsilon(1e-4));
}

CATCH_TEST_CASE("dynamic, normalize1")
{
  cml::vectord v1 = { 1., 1., 1. };
  v1.normalize();
  double l2 = v1.length_squared();
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic, normalize2")
{
  cml::vectord v1 = { 1., 1., 1. };
  double l2 = cml::normalize(v1).length_squared();
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic, normalize3")
{
  const cml::vectord v1 = { 1., 1., 1. };
  auto xpr = v1.normalize();
  double l2 = xpr.length_squared();
  CATCH_REQUIRE(v1.size() == 3);
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic, zero1")
{
  cml::vectord v1 = { 1., 1., 1. };
  v1.zero();
  CATCH_CHECK(v1[0] == 0.);
  CATCH_CHECK(v1[1] == 0.);
  CATCH_CHECK(v1[2] == 0.);
  CATCH_CHECK(v1.length() == 0.);
}

CATCH_TEST_CASE("dynamic, minimize1")
{
  cml::vectord v1 = { 1., 1., 1. };
  cml::vectord v2 = { 2., 0., 3. };
  v1.minimize(v2);
  CATCH_CHECK(v1[0] == 1.);
  CATCH_CHECK(v1[1] == 0.);
  CATCH_CHECK(v1[2] == 1.);
}

CATCH_TEST_CASE("dynamic, maximize1")
{
  cml::vectord v1 = { 1., 1., 1. };
  cml::vectord v2 = { 2., 0., 3. };
  v1.maximize(v2);
  CATCH_CHECK(v1[0] == 2.);
  CATCH_CHECK(v1[1] == 1.);
  CATCH_CHECK(v1[2] == 3.);
}

CATCH_TEST_CASE("dynamic, cardinal1")
{
  cml::vectord v1(3);
  v1.cardinal(0);
  CATCH_CHECK(v1[0] == 1.);
  CATCH_CHECK(v1[1] == 0.);
  CATCH_CHECK(v1[2] == 0.);
}

CATCH_TEST_CASE("dynamic, random1")
{
  cml::vectord v1(4);
  v1.random(0.,1.);
  for(const auto& e : v1) {
    CATCH_CHECK(e >= 0.);
    CATCH_CHECK(e < 1.);
  }
}




CATCH_TEST_CASE("dynamic external, length_squared1")
{
  double av1[] = { 1., 1., 1. };
  cml::externalnd v1(av1, 3);
  double l2 = v1.length_squared();
  CATCH_CHECK(l2 == 3.);
}

CATCH_TEST_CASE("dynamic external, length1")
{
  double av1[] = { 1., 1., 1. };
  cml::externalnd v1(av1, 3);
  double l = v1.length();
  CATCH_CHECK(l == Approx(std::sqrt(3.)).epsilon(1e-4));
}

CATCH_TEST_CASE("dynamic external, normalize1")
{
  double av1[] = { 1., 1., 1. };
  cml::externalnd v1(av1, 3);
  double l2 = v1.normalize().length_squared();
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic external, normalize2")
{
  double av1[] = { 1., 1., 1. };
  cml::externalnd v1(av1, 3);
  double l2 = cml::normalize(v1).length_squared();
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic external, zero1")
{
  double av1[] = { 1., 1., 1. };
  cml::externalnd v1(av1, 3);
  v1.zero();
  CATCH_CHECK(v1[0] == 0.);
  CATCH_CHECK(v1[1] == 0.);
  CATCH_CHECK(v1[2] == 0.);
  CATCH_CHECK(v1.length() == 0.);
}

CATCH_TEST_CASE("dynamic external, minimize1")
{
  double av1[] = { 1., 1., 1. };
  double av2[] = { 2., 0., 3. };
  cml::externalnd v1(av1, 3);
  cml::externalnd v2(av2, 3);
  v1.minimize(v2);
  CATCH_CHECK(v1[0] == 1.);
  CATCH_CHECK(v1[1] == 0.);
  CATCH_CHECK(v1[2] == 1.);
}

CATCH_TEST_CASE("dynamic external, maximize1")
{
  double av1[] = { 1., 1., 1. };
  double av2[] = { 2., 0., 3. };
  cml::externalnd v1(av1, 3);
  cml::externalnd v2(av2, 3);
  v1.maximize(v2);
  CATCH_CHECK(v1[0] == 2.);
  CATCH_CHECK(v1[1] == 1.);
  CATCH_CHECK(v1[2] == 3.);
}

CATCH_TEST_CASE("dynamic external, cardinal1")
{
  double av1[3];
  cml::externalnd v1(av1, 3);
  v1.cardinal(0);
  CATCH_CHECK(v1[0] == 1.);
  CATCH_CHECK(v1[1] == 0.);
  CATCH_CHECK(v1[2] == 0.);
}

CATCH_TEST_CASE("dynamic external, random1")
{
  double av1[4];
  cml::externalnd v1(av1, 4);
  v1.random(0.,1.);
  for(const auto& e : v1) {
    CATCH_CHECK(e >= 0.);
    CATCH_CHECK(e < 1.);
  }
}




CATCH_TEST_CASE("dynamic const external, length_squared1")
{
  const double av1[] = { 1., 1., 1. };
  cml::externalncd v1(av1, 3);
  double l2 = v1.length_squared();
  CATCH_CHECK(l2 == 3.);
}

CATCH_TEST_CASE("dynamic const external, length1")
{
  const double av1[] = { 1., 1., 1. };
  cml::externalncd v1(av1, 3);
  double l = v1.length();
  CATCH_CHECK(l == Approx(std::sqrt(3.)).epsilon(1e-4));
}

CATCH_TEST_CASE("dynamic const external, normalize1")
{
  const double av1[] = { 1., 1., 1. };
  cml::externalncd v1(av1, 3);
  double l2 = v1.normalize().length_squared();
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic const external, normalize2")
{
  const double av1[] = { 1., 1., 1. };
  cml::externalncd v1(av1, 3);
  double l2 = cml::normalize(v1).length_squared();
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}



#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS

CATCH_TEST_CASE("rv from this1, normalize1")
{
  CATCH_CHECK((std::is_rvalue_reference<decltype(
	cml::vector3d(1., 1., 1.).normalize())>::value));
  auto xpr = cml::vector3d(1., 1., 1.).normalize();
  double l2 = xpr.length_squared();
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}

CATCH_TEST_CASE("rv from this1, zero1")
{
  CATCH_CHECK((std::is_rvalue_reference<decltype(
	cml::vector3d(1., 1., 1.).zero())>::value));
  auto xpr = cml::vector3d(1., 1., 1.).zero();
  CATCH_CHECK(xpr[0] == 0.);
  CATCH_CHECK(xpr[1] == 0.);
  CATCH_CHECK(xpr[2] == 0.);
  CATCH_CHECK(xpr.length() == 0.);
}

CATCH_TEST_CASE("rv from this1, minimize1")
{
  CATCH_CHECK((std::is_rvalue_reference<decltype(
	cml::vector3d(1., 1., 1.).minimize(cml::vector3d(2., 0., 3.)))>::value));
  auto xpr = cml::vector3d(1., 1., 1.)
    .minimize(cml::vector3d(2., 0., 3.));
  CATCH_CHECK(xpr[0] == 1.);
  CATCH_CHECK(xpr[1] == 0.);
  CATCH_CHECK(xpr[2] == 1.);
}

CATCH_TEST_CASE("rv from this1, maximize1")
{
  CATCH_CHECK((std::is_rvalue_reference<decltype(
    cml::vector3d(1., 1., 1.).maximize(cml::vector3d(2., 0., 3.)))>::value));
  auto xpr = cml::vector3d(1., 1., 1.)
    .maximize(cml::vector3d(2., 0., 3.));
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 1.);
  CATCH_CHECK(xpr[2] == 3.);
}

CATCH_TEST_CASE("rv from this1, cardinal1")
{
  CATCH_CHECK((std::is_rvalue_reference<
    decltype(cml::vector3d().cardinal(0))>::value));
  auto xpr = cml::vector3d().cardinal(0);
  CATCH_CHECK(xpr[0] == 1.);
  CATCH_CHECK(xpr[1] == 0.);
  CATCH_CHECK(xpr[2] == 0.);
}

#else

CATCH_TEST_CASE("rv from this1, normalize1")
{
  CATCH_CHECK((std::is_lvalue_reference<
    decltype(cml::vector3d(1., 1., 1.).normalize())>::value));
  auto xpr = cml::vector3d(1., 1., 1.).normalize();
  CATCH_CHECK((std::is_reference<decltype(xpr)>::value) == false);

  double l2 = xpr.length_squared();
  CATCH_CHECK(l2 == Approx(1.0).epsilon(1e-12));
}

CATCH_TEST_CASE("rv from this1, zero1")
{
  CATCH_CHECK((std::is_lvalue_reference<
    decltype(cml::vector3d(1., 1., 1.).zero())>::value));
  auto xpr = cml::vector3d(1., 1., 1.).zero();
  CATCH_CHECK((std::is_reference<decltype(xpr)>::value) == false);
  CATCH_CHECK(xpr[0] == 0.);
  CATCH_CHECK(xpr[1] == 0.);
  CATCH_CHECK(xpr[2] == 0.);
  CATCH_CHECK(xpr.length() == 0.);
}

CATCH_TEST_CASE("rv from this1, minimize1")
{
  CATCH_CHECK((std::is_lvalue_reference<decltype(
	cml::vector3d(1., 1., 1.).minimize(cml::vector3d(2., 0., 3.)))>::value));
  auto xpr = cml::vector3d(1., 1., 1.)
    .minimize(cml::vector3d(2., 0., 3.));
  CATCH_CHECK((std::is_reference<decltype(xpr)>::value) == false);
  CATCH_CHECK(xpr[0] == 1.);
  CATCH_CHECK(xpr[1] == 0.);
  CATCH_CHECK(xpr[2] == 1.);
}

CATCH_TEST_CASE("rv from this1, maximize1")
{
  CATCH_CHECK((std::is_lvalue_reference<decltype(
    cml::vector3d(1., 1., 1.).maximize(cml::vector3d(2., 0., 3.)))>::value));
  auto xpr = cml::vector3d(1., 1., 1.)
    .maximize(cml::vector3d(2., 0., 3.));
  CATCH_CHECK((std::is_reference<decltype(xpr)>::value) == false);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 1.);
  CATCH_CHECK(xpr[2] == 3.);
}

CATCH_TEST_CASE("rv from this1, cardinal1")
{
  CATCH_CHECK((std::is_lvalue_reference<
    decltype(cml::vector3d().cardinal(0))>::value));
  auto xpr = cml::vector3d().cardinal(0);
  CATCH_CHECK((std::is_reference<decltype(xpr)>::value) == false);
  CATCH_CHECK(xpr[0] == 1.);
  CATCH_CHECK(xpr[1] == 0.);
  CATCH_CHECK(xpr[2] == 0.);
}

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
