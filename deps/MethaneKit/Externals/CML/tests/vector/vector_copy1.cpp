/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <cml/vector/fixed.h>
#include <cml/vector/dynamic.h>
#include <cml/vector/external.h>
#include <cml/vector/comparison.h>
#include <cml/vector/types.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("fixed, copy_temp_fixed")
{
  cml::vector3d v = { 1., 2., 3. };
  cml::vector3d w = v;
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("fixed, copy_temp_fixed_external")
{
  double av[3] = { 1., 2., 3. };
  cml::external3d v(av);
  cml::vector3d w = v;
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("fixed, copy_temp_dynamic_external")
{
  double av[3] = { 1., 2., 3. };
  cml::externalnd v(av, 3);
  cml::vector3d w = v;
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("fixed, copy_temp_dynamic")
{
  cml::vectord v = { 1., 2., 3. };
  cml::vector3d w = v; 
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("fixed, copy_assign_fixed")
{
  cml::vector3d v = { 1., 2., 3. };
  cml::vector3d w;
  w = v;
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("fixed, copy_assign_fixed_external")
{
  double av[3] = { 1., 2., 3. };
  cml::external3d v(av);
  cml::vector3d w;
  w = v;
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("fixed, copy_assign_dynamic_external")
{
  double av[3] = { 1., 2., 3. };
  cml::externalnd v(av, 3);
  cml::vector3d w;
  w = v;
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("fixed, copy_assign_dynamic")
{
  cml::vectord v = { 1., 2., 3. };
  cml::vector3d w;
  w = v; 
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("fixed, move_assign")
{
  cml::vector3d v = { 1., 2., 3. };
  cml::vector3d w;
  w = cml::vector3d(1., 2., 3.);
  CATCH_CHECK(v == w);
}




CATCH_TEST_CASE("fixed external, copy_assign_fixed")
{
  cml::vector3d v = { 1., 2., 3. };
  double aw[3];
  cml::external3d w(aw);
  w = v;
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("fixed external, copy_assign_fixed_external")
{
  double av[3] = { 1., 2., 3. };
  cml::external3d v(av);
  double aw[3];
  cml::external3d w(aw);
  w = v;
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("fixed external, copy_assign_dynamic_external")
{
  double av[3] = { 1., 2., 3. };
  cml::externalnd v(av, 3);
  double aw[3];
  cml::external3d w(aw);
  w = v;
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("fixed external, copy_assign_dynamic")
{
  cml::vectord v = { 1., 2., 3. };
  double aw[3];
  cml::external3d w(aw);
  w = v; 
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("fixed external, move_assign")
{
  cml::vector3d v = { 1., 2., 3. };
#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
  cml::external3d w;
#else
  cml::external3d w(nullptr);
  // Note: this allows the test to go through, but is not recommended for
  // user code on compilers that do not support rvalue references from
  // this.
#endif
  double at[3] = { 1., 2., 3. };
  w = cml::external3d(at);
  CATCH_REQUIRE(w.data() == &at[0]);
  CATCH_CHECK(v == w);
}




CATCH_TEST_CASE("dynamic external, copy_assign_fixed")
{
  cml::vector3d v = { 1., 2., 3. };
  double aw[3];
  cml::externalnd w(aw, 3);
  w = v;
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("dynamic external, copy_assign_fixed_external")
{
  double av[3] = { 1., 2., 3. };
  cml::external3d v(av);
  double aw[3];
  cml::externalnd w(aw, 3);
  w = v;
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("dynamic external, copy_assign_dynamic_external")
{
  double av[3] = { 1., 2., 3. };
  cml::externalnd v(av, 3);
  double aw[3];
  cml::externalnd w(aw, 3);
  w = v;
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("dynamic external, copy_assign_dynamic")
{
  cml::vectord v = { 1., 2., 3. };
  double aw[3];
  cml::externalnd w(aw, 3);
  w = v; 
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("dynamic external, move_assign")
{
  cml::vector3d v = { 1., 2., 3. };
#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
  cml::externalnd w;
#else
  cml::externalnd w(nullptr, 0);
  // Note: this allows the test to go through, but is not recommended for
  // user code on compilers that do not support rvalue references from
  // this.
#endif
  double at[3] = { 1., 2., 3. };
  w = cml::externalnd(at, 3);
  CATCH_REQUIRE(w.data() == &at[0]);
  CATCH_CHECK(v == w);
}




CATCH_TEST_CASE("dynamic, copy_temp_fixed")
{
  cml::vector3d v = { 1., 2., 3. };
  cml::vectord w = v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("dynamic, copy_temp_fixed_external")
{
  double av[3] = { 1., 2., 3. };
  cml::external3d v(av);
  cml::vectord w = v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("dynamic, copy_temp_dynamic_external")
{
  double av[3] = { 1., 2., 3. };
  cml::externalnd v(av, 3);
  cml::vectord w = v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("dynamic, copy_temp_dynamic")
{
  cml::vectord v = { 1., 2., 3. };
  cml::vectord w = v; 
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("dynamic, copy_assign_fixed")
{
  cml::vector3d v = { 1., 2., 3. };
  cml::vectord w;
  w = v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("dynamic, copy_assign_fixed_external")
{
  double av[3] = { 1., 2., 3. };
  cml::external3d v(av);
  cml::vectord w;
  w = v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("dynamic, copy_assign_dynamic_external")
{
  double av[3] = { 1., 2., 3. };
  cml::externalnd v(av, 3);
  cml::vectord w;
  w = v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("dynamic, copy_assign_dynamic")
{
  cml::vectord v = { 1., 2., 3. };
  cml::vectord w;
  w = v; 
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(v == w);
}

CATCH_TEST_CASE("dynamic, move_assign")
{
  cml::vector3d v = { 1., 2., 3. };
  cml::vectord w;
  w = cml::vectord(1., 2., 3.);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(v == w);
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
