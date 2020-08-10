/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/quaternion/unary_node.h>
#include <cml/quaternion/unary_ops.h>

#include <cml/quaternion/fixed.h>
#include <cml/quaternion/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("unary_types1")
{
  typedef cml::quaterniond quaternion_type;
  {
    CATCH_CHECK(cml::is_statically_polymorphic<quaternion_type>::value);
  }
  {
    auto xpr = - quaternion_type();
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::sub_arg_type>::value
      );
  }
  {
    auto xpr = + quaternion_type();
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::sub_arg_type>::value
      );
  }
  {
    quaternion_type M;
    auto xpr = - M;
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::sub_arg_type>::value
      );
  }
  {
    quaternion_type M;
    auto xpr = + M;
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::sub_arg_type>::value
      );
  }
}


CATCH_TEST_CASE("fixed, unary_minus1")
{
  cml::quaterniond q = { 1., 2., 3., 4. };
  cml::quaterniond r;
  r = - q;
  CATCH_REQUIRE(r.size() == 4);
  CATCH_CHECK(r[0] == - 1.);
  CATCH_CHECK(r[1] == - 2.);
  CATCH_CHECK(r[2] == - 3.);
  CATCH_CHECK(r[3] == - 4.);
}

CATCH_TEST_CASE("fixed, unary_minus2")
{
  cml::quaterniond q = { 1., 2., 3., 4. };
  cml::quaterniond r = - q;
  CATCH_REQUIRE(r.size() == 4);
  CATCH_CHECK(r[0] == - 1.);
  CATCH_CHECK(r[1] == - 2.);
  CATCH_CHECK(r[2] == - 3.);
  CATCH_CHECK(r[3] == - 4.);
}

CATCH_TEST_CASE("fixed, unary_plus1")
{
  cml::quaterniond q = { 1., 2., 3., 4. };
  cml::quaterniond r;
  r = + q;
  CATCH_REQUIRE(r.size() == 4);
  CATCH_CHECK(r[0] == + 1.);
  CATCH_CHECK(r[1] == + 2.);
  CATCH_CHECK(r[2] == + 3.);
  CATCH_CHECK(r[3] == + 4.);
}

CATCH_TEST_CASE("fixed, unary_plus2")
{
  cml::quaterniond q = { 1., 2., 3., 4. };
  cml::quaterniond r = + q;
  CATCH_REQUIRE(r.size() == 4);
  CATCH_CHECK(r[0] == + 1.);
  CATCH_CHECK(r[1] == + 2.);
  CATCH_CHECK(r[2] == + 3.);
  CATCH_CHECK(r[3] == + 4.);
}

CATCH_TEST_CASE("fixed, double_negate1")
{
  cml::quaterniond q = { 1., 2., 3., 4. };
  cml::quaterniond r = - (- q);
  CATCH_REQUIRE(r.size() == 4);
  CATCH_CHECK(r[0] == 1.);
  CATCH_CHECK(r[1] == 2.);
  CATCH_CHECK(r[2] == 3.);
  CATCH_CHECK(r[3] == 4.);
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
