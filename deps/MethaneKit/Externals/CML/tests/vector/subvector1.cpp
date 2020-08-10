/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <iostream>

// Make sure the main header compiles cleanly:
#include <cml/vector/subvector_node.h>

#include <cml/vector/fixed.h>
#include <cml/vector/dynamic.h>
#include <cml/vector/external.h>
#include <cml/vector/subvector.h>
#include <cml/vector/types.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("fixed, sub1")
{
  cml::vector3d v1 = { 1., 2., 3. };
  auto xpr = cml::subvector(v1,0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 2);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}

CATCH_TEST_CASE("fixed, sub2")
{
  cml::vector3d v1 = { 1., 2., 3. };
  auto xpr = v1.subvector(0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 2);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}

CATCH_TEST_CASE("fixed, sub3")
{
  auto xpr = cml::subvector(cml::vector3d(1., 2., 3.), 0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 2);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}




CATCH_TEST_CASE("fixed external, sub1")
{
  double av1[] = { 1., 2., 3. };
  cml::external3d v1(av1);
  auto xpr = cml::subvector(v1,0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 2);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}

CATCH_TEST_CASE("fixed external, sub2")
{
  double av1[] = { 1., 2., 3. };
  cml::external3d v1(av1);
  auto xpr = v1.subvector(0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 2);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}

CATCH_TEST_CASE("fixed external, sub3")
{
  double av1[] = { 1., 2., 3. };
  auto xpr = cml::subvector(cml::external3d(av1), 0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 2);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}




CATCH_TEST_CASE("fixed const external, sub1")
{
  const double av1[] = { 1., 2., 3. };
  cml::external3cd v1(av1);
  auto xpr = cml::subvector(v1,0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 2);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}

CATCH_TEST_CASE("fixed const external, sub2")
{
  const double av1[] = { 1., 2., 3. };
  cml::external3cd v1(av1);
  auto xpr = v1.subvector(0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 2);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}

CATCH_TEST_CASE("fixed const external, sub3")
{
  const double av1[] = { 1., 2., 3. };
  auto xpr = cml::subvector(cml::external3cd(av1), 0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 2);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}




CATCH_TEST_CASE("dynamic, sub1")
{
  cml::vectord v1 = { 1., 2., 3. };
  auto xpr = cml::subvector(v1,0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == -1);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}

CATCH_TEST_CASE("dynamic, sub2")
{
  cml::vectord v1 = { 1., 2., 3. };
  auto xpr = v1.subvector(0);
  CATCH_REQUIRE(v1.size() == 3);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == -1);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}

CATCH_TEST_CASE("dynamic, sub3")
{
  auto xpr = cml::subvector(cml::vectord(1., 2., 3.), 0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == -1);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}




CATCH_TEST_CASE("dynamic external, sub1")
{
  double av1[] = { 1., 2., 3. };
  cml::externalnd v1(av1, 3);
  auto xpr = cml::subvector(v1,0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == -1);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}

CATCH_TEST_CASE("dynamic external, sub2")
{
  double av1[] = { 1., 2., 3. };
  cml::externalnd v1(av1, 3);
  auto xpr = v1.subvector(0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == -1);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}

CATCH_TEST_CASE("dynamic external, sub3")
{
  double av1[] = { 1., 2., 3. };
  auto xpr = cml::subvector(cml::externalnd(av1, 3), 0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == -1);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}




CATCH_TEST_CASE("dynamic const external, sub1")
{
  const double av1[] = { 1., 2., 3. };
  cml::externalncd v1(av1, 3);
  auto xpr = cml::subvector(v1,0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == -1);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}

CATCH_TEST_CASE("dynamic const external, sub2")
{
  const double av1[] = { 1., 2., 3. };
  cml::externalncd v1(av1, 3);
  auto xpr = v1.subvector(0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == -1);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}

CATCH_TEST_CASE("dynamic const external, sub3")
{
  const double av1[] = { 1., 2., 3. };
  auto xpr = cml::subvector(cml::externalncd(av1, 3), 0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == -1);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}



#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS

CATCH_TEST_CASE("rv from this1, sub1")
{
  typedef decltype(cml::vector3d().subvector(0)) node_type;
  CATCH_CHECK((std::is_rvalue_reference<typename node_type::sub_arg_type>::value));
  auto xpr = cml::vector3d(1., 2., 3.).subvector(0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 2);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}

CATCH_TEST_CASE("rv from this1, sub2")
{
  cml::vector3d v1 = { 1., 2., 3. };
  typedef decltype(v1.subvector(0)) node_type;
  CATCH_CHECK((std::is_lvalue_reference<typename node_type::sub_arg_type>::value));
  auto xpr = v1.subvector(0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 2);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}

#else

CATCH_TEST_CASE("rv from this1, sub1")
{
  typedef cml::vector3d vector3_t;
  typedef decltype(vector3_t().subvector(0)) node_type;

  CATCH_CHECK((std::is_same<typename node_type::sub_type, vector3_t>::value));

  auto xpr = vector3_t(1., 2., 3.).subvector(0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 2);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}

CATCH_TEST_CASE("rv from this1, sub2")
{
  typedef cml::vector3d vector3_t;
  vector3_t v1 = { 1., 2., 3. };
  typedef decltype(v1.subvector(0)) node_type;

  CATCH_CHECK((std::is_same<typename node_type::sub_type, vector3_t>::value));

  auto xpr = v1.subvector(0);
  CATCH_REQUIRE(cml::int_c<decltype(xpr)::array_size>::value == 2);
  CATCH_REQUIRE(xpr.size() == 2);
  CATCH_CHECK(xpr[0] == 2.);
  CATCH_CHECK(xpr[1] == 3.);
}

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
