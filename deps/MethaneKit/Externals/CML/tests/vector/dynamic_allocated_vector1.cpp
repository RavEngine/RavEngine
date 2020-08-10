/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <iostream>

#include <cml/vector/dynamic_allocated.h>
#include <cml/vector/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("alloc1")
{
  cml::vectord v(3);
  CATCH_REQUIRE(v.size() == 3);
}

CATCH_TEST_CASE("alloc2")
{
  cml::vectord v(3U);
  CATCH_REQUIRE(v.size() == 3);
}

CATCH_TEST_CASE("alloc3")
{
  cml::vectord v(3L);
  CATCH_REQUIRE(v.size() == 3);
}

CATCH_TEST_CASE("alloc4")
{
  cml::vectord v(3UL);
  CATCH_REQUIRE(v.size() == 3);
}

CATCH_TEST_CASE("resize1")
{
  cml::vectord v(3);
  CATCH_REQUIRE(v.size() == 3);
  v.resize(5);
  CATCH_REQUIRE(v.size() == 5);
}

CATCH_TEST_CASE("array_construct")
{
  double data[] = { 1., 2., 3. };
  cml::vectord v(data);
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("array_temp_construct")
{
  double data[] = { 1., 2., 3. };
  cml::vectord v = data;
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("array_assign")
{
  double data[] = { 1., 2., 3. };
  cml::vectord v;
  v = data;
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("element_construct1")
{
  cml::vectord v(1.);
  CATCH_REQUIRE(v.size() == 1);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("element_construct2")
{
  cml::vectord v(1., 2.);
  CATCH_REQUIRE(v.size() == 2);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
  CATCH_CHECK(v[1] == 2.);
}

CATCH_TEST_CASE("element_construct3")
{
  cml::vectord v(1., 2., 3.);
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
  CATCH_CHECK(v[1] == 2.);
  CATCH_CHECK(v[2] == 3.);
}

CATCH_TEST_CASE("element_construct4")
{
  cml::vectord v(1.,2.f,3,4U);
  CATCH_REQUIRE(v.size() == 4);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
  CATCH_CHECK(v[1] == 2.);
  CATCH_CHECK(v[2] == 3.);
  CATCH_CHECK(v[3] == 4.);
}

CATCH_TEST_CASE("combine_construct1")
{
  cml::vectord v(cml::vectord(1., 2., 3.), 4.);
  CATCH_REQUIRE(v.size() == 4);
  CATCH_CHECK(v[0] == 1.);
  CATCH_CHECK(v[1] == 2.);
  CATCH_CHECK(v[2] == 3.);
  CATCH_CHECK(v[3] == 4.);
}

CATCH_TEST_CASE("combine_construct2")
{
  cml::vectord v(cml::vectord(1., 2.), 3., 4.);
  CATCH_REQUIRE(v.size() == 4);
  CATCH_CHECK(v[0] == 1.);
  CATCH_CHECK(v[1] == 2.);
  CATCH_CHECK(v[2] == 3.);
  CATCH_CHECK(v[3] == 4.);
}

CATCH_TEST_CASE("pointer_construct1")
{
  double data[] = { 1., 2., 3. };
  cml::vectord v(3, &data[0]);
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("pointer_construct2")
{
  double data[] = { 1., 2., 3. };
  cml::vectord v(&data[0], 3);
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("list_construct")
{
  cml::vectord v { 1., 2., 3. };
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("list_temp_construct")
{
  cml::vectord v = { 1., 2., 3. };
  CATCH_REQUIRE(v.size() == 3);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("fill1")
{
  cml::vectord v(5);
  v.fill(1.);
  CATCH_REQUIRE(v.size() == 5);
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[4] == 1.);
}

CATCH_TEST_CASE("write1")
{
  cml::vectord v(3);
  CATCH_REQUIRE(v.size() == 3);
  v[0] = 1.;
  CATCH_CHECK(v.data()[0] == 1.);
  CATCH_CHECK(v[0] == 1.);
}

CATCH_TEST_CASE("size_check1")
{
  cml::vectord v;
  CATCH_REQUIRE(v.size() == 0);
  CATCH_CHECK_NOTHROW((v = { 1., 2., 3., 4. }));
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
