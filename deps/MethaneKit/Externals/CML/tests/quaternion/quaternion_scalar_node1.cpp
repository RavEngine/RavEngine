/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/quaternion/scalar_node.h>
#include <cml/quaternion/scalar_ops.h>

#include <cml/quaternion/fixed.h>
#include <cml/quaternion/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("scalar_types1")
{
  typedef cml::quaterniond quaternion_type;
  {
    auto xpr = quaternion_type()*int();
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::right_arg_type>::value
      );
    CATCH_CHECK(
      std::is_arithmetic<typename xpr_type::right_type>::value
      );
  }
  {
    auto xpr = int()*quaternion_type();
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::right_arg_type>::value
      );
    CATCH_CHECK(
      std::is_arithmetic<typename xpr_type::right_type>::value
      );
  }
  {
    auto xpr = quaternion_type()/int();
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::right_arg_type>::value
      );
    CATCH_CHECK(
      std::is_arithmetic<typename xpr_type::right_type>::value
      );
  }
  {
    double v = 0.;
    auto xpr = quaternion_type()*v;
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::right_arg_type>::value
      );
    CATCH_CHECK(
      std::is_arithmetic<typename xpr_type::right_type>::value
      );
  }
  {
    double v = 0.;
    auto xpr = v*quaternion_type();
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::right_arg_type>::value
      );
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_arithmetic<typename xpr_type::right_type>::value
      );
  }
  {
    double v = 0.;
    auto xpr = quaternion_type()/v;
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::right_arg_type>::value
      );
    CATCH_CHECK(
      std::is_arithmetic<typename xpr_type::right_type>::value
      );
  }
}


CATCH_TEST_CASE("fixed, scalar_multiply1")
{
  cml::quaterniond q = { 1., 2., 3., 4. };
  cml::quaterniond r;
  r = 2.*q;
  CATCH_REQUIRE(r.size() == 4);
  CATCH_CHECK(r[0] == 2.);
  CATCH_CHECK(r[1] == 4.);
  CATCH_CHECK(r[2] == 6.);
  CATCH_CHECK(r[3] == 8.);
}

CATCH_TEST_CASE("fixed, scalar_multiply2")
{
  cml::quaterniond q = { 1., 2., 3., 4. };
  cml::quaterniond r = 2.*q;
  CATCH_REQUIRE(r.size() == 4);
  CATCH_CHECK(r[0] == 2.);
  CATCH_CHECK(r[1] == 4.);
  CATCH_CHECK(r[2] == 6.);
  CATCH_CHECK(r[3] == 8.);
}

CATCH_TEST_CASE("fixed, scalar_divide1")
{
  cml::quaterniond q = { 2., 4., 6., 8. };
  cml::quaterniond r;
  r = q/2;
  CATCH_REQUIRE(r.size() == 4);
  CATCH_CHECK(r[0] == 1.);
  CATCH_CHECK(r[1] == 2.);
  CATCH_CHECK(r[2] == 3.);
  CATCH_CHECK(r[3] == 4.);
}

CATCH_TEST_CASE("fixed, scalar_divide2")
{
  cml::quaterniond q = { 2., 4., 6., 8. };
  cml::quaterniond r = q/2;
  CATCH_REQUIRE(r.size() == 4);
  CATCH_CHECK(r[0] == 1.);
  CATCH_CHECK(r[1] == 2.);
  CATCH_CHECK(r[2] == 3.);
  CATCH_CHECK(r[3] == 4.);
}

CATCH_TEST_CASE("fixed, scalar_multiply_assign1")
{
  cml::quaterniond q = { 1., 2., 3., 4. };
  q *= 2;
  CATCH_REQUIRE(q.size() == 4);
  CATCH_CHECK(q[0] == 2.);
  CATCH_CHECK(q[1] == 4.);
  CATCH_CHECK(q[2] == 6.);
  CATCH_CHECK(q[3] == 8.);
}

CATCH_TEST_CASE("fixed, scalar_multiply_assign2")
{
  cml::quaterniond q;
  q = (cml::quaterniond(1., 2., 3., 4.) *= 2);
  CATCH_REQUIRE(q.size() == 4);
  CATCH_CHECK(q[0] == 2.);
  CATCH_CHECK(q[1] == 4.);
  CATCH_CHECK(q[2] == 6.);
  CATCH_CHECK(q[3] == 8.);
}



// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
