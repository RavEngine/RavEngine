/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/vector/unary_node.h>
#include <cml/vector/unary_ops.h>

#include <cml/vector/fixed.h>
#include <cml/vector/external.h>
#include <cml/vector/dynamic.h>
#include <cml/vector/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("unary_types1")
{
  typedef cml::vector3d vector_type;
  {
    CATCH_CHECK(cml::is_statically_polymorphic<vector_type>::value);
  }
  {
    auto xpr = - vector_type();
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::sub_arg_type>::value
      );
  }
  {
    auto xpr = + vector_type();
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::sub_arg_type>::value
      );
  }
  {
    vector_type M;
    auto xpr = - M;
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::sub_arg_type>::value
      );
  }
  {
    vector_type M;
    auto xpr = + M;
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::sub_arg_type>::value
      );
  }
}



CATCH_TEST_CASE("fixed, unary_minus1")
{
  cml::vector3d v = { 1., 2., 3. };
  cml::vector3d w;
  w = - v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == - 1.);
}

CATCH_TEST_CASE("fixed, unary_minus2")
{
  cml::vector3d v = { 1., 2., 3. };
  cml::vector3d w = - v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == - 1.);
}

CATCH_TEST_CASE("fixed, unary_plus1")
{
  cml::vector3d v = { 1., 2., 3. };
  cml::vector3d w;
  w = + v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 1.);
}

CATCH_TEST_CASE("fixed, unary_plus2")
{
  cml::vector3d v = { 1., 2., 3. };
  cml::vector3d w = + v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 1.);
}

CATCH_TEST_CASE("fixed, double_negate1")
{
  cml::vector3d v = { 1., 2., 3. };
  CATCH_REQUIRE(v.size() == 3);

  cml::vector3d w;
  CATCH_REQUIRE(w.size() == 3);
  auto xpr = - (-v);
  w = xpr;
  CATCH_CHECK(w[0] == 1.);
}




CATCH_TEST_CASE("fixed external, unary_minus1")
{
  double av[3] = { 1., 2., 3. };
  cml::external3d v(av);

  double aw[3];
  cml::external3d w(aw);

  w = - v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == - 1.);
}

CATCH_TEST_CASE("fixed external, unary_plus1")
{
  double av[3] = { 1., 2., 3. };
  cml::external3d v(av);

  double aw[3];
  cml::external3d w(aw);

  w = + v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 1.);
}

CATCH_TEST_CASE("fixed external, double_negate1")
{
  double av[3] = { 1., 2., 3. };
  cml::external3d v(av);
  CATCH_REQUIRE(v.size() == 3);

  double aw[3];
  cml::external3d w(aw);
  CATCH_REQUIRE(w.size() == 3);

  auto xpr = - (-v);
  w = xpr;
  CATCH_CHECK(w[0] == 1.);
}




CATCH_TEST_CASE("dynamic external, unary_minus1")
{
  double av[3] = { 1., 2., 3. };
  cml::externalnd v(av,3);

  double aw[3];
  cml::externalnd w(aw,3);

  w = - v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == - 1.);
}

CATCH_TEST_CASE("dynamic external, unary_plus1")
{
  double av[3] = { 1., 2., 3. };
  cml::externalnd v(av,3);

  double aw[3];
  cml::externalnd w(aw,3);

  w = + v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 1.);
}

CATCH_TEST_CASE("dynamic external, double_negate1")
{
  double av[3] = { 1., 2., 3. };
  cml::externalnd v(av,3);
  CATCH_REQUIRE(v.size() == 3);

  double aw[3];
  cml::externalnd w(aw,3);
  CATCH_REQUIRE(w.size() == 3);

  auto xpr = - (-v);
  w = xpr;
  CATCH_CHECK(w[0] == 1.);
}




CATCH_TEST_CASE("dynamic, unary_minus1")
{
  cml::vectord v = { 1., 2., 3. };
  cml::vectord w;
  w = - v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == - 1.);
}

CATCH_TEST_CASE("dynamic, unary_minus2")
{
  cml::vectord v = { 1., 2., 3. };
  cml::vectord w = - v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == - 1.);
}

CATCH_TEST_CASE("dynamic, unary_plus1")
{
  cml::vectord v = { 1., 2., 3. };
  cml::vectord w;
  w = + v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 1.);
}

CATCH_TEST_CASE("dynamic, unary_plus2")
{
  cml::vectord v = { 1., 2., 3. };
  cml::vectord w = + v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 1.);
}

CATCH_TEST_CASE("dynamic, double_negate1")
{
  cml::vectord v = { 1., 2., 3. };
  cml::vectord w;
  CATCH_REQUIRE(v.size() == 3);
  auto xpr = - (-v);
  w = xpr;
  CATCH_CHECK(w[0] == 1.);
}




CATCH_TEST_CASE("dynamic const external, unary_minus1")
{
  const double av[3] = { 1., 2., 3. };
  cml::externalncd v(av,3);

  double aw[3];
  cml::externalnd w(aw,3);

  w = - v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == - 1.);
}

CATCH_TEST_CASE("dynamic const external, unary_plus1")
{
  const double av[3] = { 1., 2., 3. };
  cml::externalncd v(av,3);

  double aw[3];
  cml::externalnd w(aw,3);

  w = + v;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 1.);
}

CATCH_TEST_CASE("dynamic const external, double_negate1")
{
  const double av[3] = { 1., 2., 3. };
  cml::externalncd v(av,3);
  CATCH_REQUIRE(v.size() == 3);

  double aw[3];
  cml::externalnd w(aw,3);
  CATCH_REQUIRE(w.size() == 3);

  auto xpr = - (-v);
  w = xpr;
  CATCH_CHECK(w[0] == 1.);
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
