/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/quaternion/binary_node.h>
#include <cml/quaternion/binary_ops.h>

#include <cml/quaternion/fixed.h>
#include <cml/quaternion/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("binary_types1")
{
  typedef cml::quaterniond quaternion_type;
  {
    CATCH_CHECK(cml::is_statically_polymorphic<quaternion_type>::value);
  }
  {
    auto xpr = quaternion_type() + quaternion_type();
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::right_arg_type>::value
      );
  }
  {
    auto xpr = quaternion_type() - quaternion_type();
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::right_arg_type>::value
      );
  }
  {
    quaternion_type M;
    auto xpr = quaternion_type() + M;
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::right_arg_type>::value
      );
  }
  {
    quaternion_type M;
    auto xpr = M + quaternion_type();
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::right_arg_type>::value
      );
  }
  {
    quaternion_type M;
    auto xpr = quaternion_type() - M;
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::right_arg_type>::value
      );
  }
  {
    quaternion_type M;
    auto xpr = M - quaternion_type();
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::right_arg_type>::value
      );
  }
  {
    quaternion_type M1, M2;
    auto xpr = M1 + M2;
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::right_arg_type>::value
      );
  }
  {
    quaternion_type M1, M2;
    auto xpr = M1 - M2;
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::right_arg_type>::value
      );
  }
}



CATCH_TEST_CASE("fixed, binary_minus1")
{
  cml::quaterniond q = { 1., 2., 3., 4. };
  cml::quaterniond r = { 4., 5., 6., 7. };
  cml::quaterniond w;
  w = q - r;
  CATCH_REQUIRE(w.size() == 4);
  CATCH_CHECK(w[0] == -3.);
  CATCH_CHECK(w[1] == -3.);
  CATCH_CHECK(w[2] == -3.);
  CATCH_CHECK(w[3] == -3.);
}

CATCH_TEST_CASE("fixed, binary_minus2")
{
  cml::quaterniond q = { 1., 2., 3., 4. };
  cml::quaterniond r = { 4., 5., 6., 7. };
  cml::quaterniond w = q - r;
  CATCH_REQUIRE(w.size() == 4);
  CATCH_CHECK(w[0] == -3.);
  CATCH_CHECK(w[1] == -3.);
  CATCH_CHECK(w[2] == -3.);
  CATCH_CHECK(w[3] == -3.);
}

CATCH_TEST_CASE("fixed, binary_plus1")
{
  cml::quaterniond q = { 1., 2., 3., 4. };
  cml::quaterniond r = { 4., 5., 6., 7. };
  cml::quaterniond w;
  w = q + r;
  CATCH_REQUIRE(w.size() == 4);
  CATCH_CHECK(w[0] == 5.);
  CATCH_CHECK(w[1] == 7.);
  CATCH_CHECK(w[2] == 9.);
  CATCH_CHECK(w[3] == 11.);
}

CATCH_TEST_CASE("fixed, binary_plus2")
{
  cml::quaterniond q = { 1., 2., 3., 4. };
  cml::quaterniond r = { 4., 5., 6., 7. };
  cml::quaterniond w = q + r;
  CATCH_REQUIRE(w.size() == 4);
  CATCH_CHECK(w[0] == 5.);
  CATCH_CHECK(w[1] == 7.);
  CATCH_CHECK(w[2] == 9.);
  CATCH_CHECK(w[3] == 11.);
}

CATCH_TEST_CASE("fixed, multiple_plus1")
{
  cml::quaterniond q = { 1., 2., 3., 1. };
  cml::quaterniond r = { 4., 5., 6., 2. };
  cml::quaterniond s = { 7., 8., 9., 3. };
  cml::quaterniond w;
  w = q + (r + s);
  CATCH_REQUIRE(w.size() == 4);
  CATCH_CHECK(w[0] == 12.);
  CATCH_CHECK(w[1] == 15.);
  CATCH_CHECK(w[2] == 18.);
  CATCH_CHECK(w[3] == 6.);
}

CATCH_TEST_CASE("fixed, multiple_plus2")
{
  cml::quaterniond q = { 1., 2., 3., 1. };
  cml::quaterniond r = { 4., 5., 6., 2. };
  cml::quaterniond s = { 7., 8., 9., 3. };
  cml::quaterniond w = q + (r + s);
  CATCH_REQUIRE(w.size() == 4);
  CATCH_CHECK(w[0] == 12.);
  CATCH_CHECK(w[1] == 15.);
  CATCH_CHECK(w[2] == 18.);
  CATCH_CHECK(w[3] == 6.);
}

CATCH_TEST_CASE("fixed, mixed_op1")
{
  cml::quaterniond q = { 1., 2., 3., 1. };
  cml::quaterniond r = { 4., 5., 6., 2. };
  cml::quaterniond s = { 7., 8., 9., 3. };
  cml::quaterniond w;
  w = r - (q + s);
  CATCH_REQUIRE(w.size() == 4);
  CATCH_CHECK(w[0] == -4.);
  CATCH_CHECK(w[1] == -5.);
  CATCH_CHECK(w[2] == -6.);
  CATCH_CHECK(w[3] == -2.);
}

CATCH_TEST_CASE("fixed, mixed_op2")
{
  cml::quaterniond q = { 1., 2., 3., 1. };
  cml::quaterniond r = { 4., 5., 6., 2. };
  cml::quaterniond s = { 7., 8., 9., 3. };
  cml::quaterniond w = r - (q + s);
  CATCH_REQUIRE(w.size() == 4);
  CATCH_CHECK(w[0] == -4.);
  CATCH_CHECK(w[1] == -5.);
  CATCH_CHECK(w[2] == -6.);
  CATCH_CHECK(w[3] == -2.);
}

CATCH_TEST_CASE("fixed, assign_minus1")
{
  cml::quaterniond w = { 1., 2., 3., 4. };
  cml::quaterniond v = { 4., 5., 6., 7. };
  w -= v;
  CATCH_CHECK(w[0] == -3.);
  CATCH_CHECK(w[1] == -3.);
  CATCH_CHECK(w[2] == -3.);
  CATCH_CHECK(w[3] == -3.);
}

CATCH_TEST_CASE("fixed, assign_plus1")
{
  cml::quaterniond w = { 1., 2., 3., 4. };
  cml::quaterniond v = { 4., 5., 6., 7. };
  w += v;
  CATCH_CHECK(w[0] == 5.);
  CATCH_CHECK(w[1] == 7.);
  CATCH_CHECK(w[2] == 9.);
  CATCH_CHECK(w[3] == 11.);
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
