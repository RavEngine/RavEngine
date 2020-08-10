/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/matrix/unary_node.h>
#include <cml/matrix/unary_ops.h>

#include <cml/matrix/fixed.h>
#include <cml/matrix/external.h>
#include <cml/matrix/dynamic.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("unary_types1")
{
  typedef cml::matrix<double, cml::fixed<2,2>> matrix_type;
  {
    CATCH_CHECK(cml::is_statically_polymorphic<matrix_type>::value);
  }
  {
    auto xpr = - matrix_type();
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::sub_arg_type>::value
      );
  }
  {
    auto xpr = + matrix_type();
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::sub_arg_type>::value
      );
  }
  {
    matrix_type M;
    auto xpr = - M;
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::sub_arg_type>::value
      );
  }
  {
    matrix_type M;
    auto xpr = + M;
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::sub_arg_type>::value
      );
  }
}

CATCH_TEST_CASE("fixed unary_minus1")
{
  cml::matrix<double, cml::fixed<2,2>> M1(
    1., 2.,
    3., 4.
    );
  cml::matrix<double, cml::fixed<2,2>> M;
  M = - M1;
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == -1.);
  CATCH_CHECK(M(0,1) == -2.);
  CATCH_CHECK(M(1,0) == -3.);
  CATCH_CHECK(M(1,1) == -4.);
}

CATCH_TEST_CASE("fixed unary_minus2")
{
  cml::matrix<double, cml::fixed<2,2>> M1(
    1., 2.,
    3., 4.
    );
  cml::matrix<double, cml::fixed<2,2>> M = - M1;
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == -1.);
  CATCH_CHECK(M(0,1) == -2.);
  CATCH_CHECK(M(1,0) == -3.);
  CATCH_CHECK(M(1,1) == -4.);
}

CATCH_TEST_CASE("fixed unary_plus1")
{
  cml::matrix<double, cml::fixed<2,2>> M1(
    1., 2.,
    3., 4.
    );
  cml::matrix<double, cml::fixed<2,2>> M;
  M = + M1;
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(0,1) == 2.);
  CATCH_CHECK(M(1,0) == 3.);
  CATCH_CHECK(M(1,1) == 4.);
}

CATCH_TEST_CASE("fixed unary_plus2")
{
  cml::matrix<double, cml::fixed<2,2>> M1(
    1., 2.,
    3., 4.
    );
  cml::matrix<double, cml::fixed<2,2>> M = + M1;
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(0,1) == 2.);
  CATCH_CHECK(M(1,0) == 3.);
  CATCH_CHECK(M(1,1) == 4.);
}

CATCH_TEST_CASE("fixed double_negate1")
{
  cml::matrix<double, cml::fixed<2,2>> M1(
    1., 2.,
    3., 4.
    );
  cml::matrix<double, cml::fixed<2,2>> M;
  auto xpr = - (-M1);
  M = xpr;
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(0,1) == 2.);
  CATCH_CHECK(M(1,0) == 3.);
  CATCH_CHECK(M(1,1) == 4.);
}

CATCH_TEST_CASE("fixed external unary_minus1")
{
  double aM1[] = {
    1., 2.,
    3., 4.
  };
  cml::matrix<double, cml::external<2,2>> M1(aM1);

  double data[2][2];
  cml::matrix<double, cml::external<2,2>> M(data);
  M = - M1;

  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == -1.);
  CATCH_CHECK(M(0,1) == -2.);
  CATCH_CHECK(M(1,0) == -3.);
  CATCH_CHECK(M(1,1) == -4.);
}

CATCH_TEST_CASE("fixed external unary_plus1")
{
  double aM1[] = {
    1., 2.,
    3., 4.
  };
  cml::matrix<double, cml::external<2,2>> M1(aM1);

  double data[2][2];
  cml::matrix<double, cml::external<2,2>> M(data);
  M = + M1;

  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(0,1) == 2.);
  CATCH_CHECK(M(1,0) == 3.);
  CATCH_CHECK(M(1,1) == 4.);
}

CATCH_TEST_CASE("fixed external double_negate1")
{
  double aM1[] = {
    1., 2.,
    3., 4.
  };
  cml::matrix<double, cml::external<2,2>> M1(aM1);

  double data[2][2];
  cml::matrix<double, cml::external<2,2>> M(data);

  auto xpr = - (-M1);
  M = xpr;

  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(0,1) == 2.);
  CATCH_CHECK(M(1,0) == 3.);
  CATCH_CHECK(M(1,1) == 4.);
}

CATCH_TEST_CASE("dynamic external unary_minus1")
{
  double aM1[] = {
    1., 2.,
    3., 4.
  };
  cml::matrix<double, cml::external<>> M1(aM1, 2,2);

  double data[2][2];
  cml::matrix<double, cml::external<>> M(data);
  M = - M1;

  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == -1.);
  CATCH_CHECK(M(0,1) == -2.);
  CATCH_CHECK(M(1,0) == -3.);
  CATCH_CHECK(M(1,1) == -4.);
}

CATCH_TEST_CASE("dynamic external unary_plus1")
{
  double aM1[] = {
    1., 2.,
    3., 4.
  };
  cml::matrix<double, cml::external<>> M1(aM1, 2,2);

  double data[2][2];
  cml::matrix<double, cml::external<>> M(data);
  M = + M1;

  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(0,1) == 2.);
  CATCH_CHECK(M(1,0) == 3.);
  CATCH_CHECK(M(1,1) == 4.);
}

CATCH_TEST_CASE("dynamic external double_negate1")
{
  double aM1[] = {
    1., 2.,
    3., 4.
  };
  cml::matrix<double, cml::external<>> M1(aM1, 2,2);

  double data[2][2];
  cml::matrix<double, cml::external<>> M(data);

  auto xpr = - (-M1);
  M = xpr;

  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(0,1) == 2.);
  CATCH_CHECK(M(1,0) == 3.);
  CATCH_CHECK(M(1,1) == 4.);
}

CATCH_TEST_CASE("dynamic unary_minus1")
{
  cml::matrix<double, cml::dynamic<>> M1(
    2,2,
    1., 2.,
    3., 4.
    );
  cml::matrix<double, cml::dynamic<>> M;
  M = - M1;
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == -1.);
  CATCH_CHECK(M(0,1) == -2.);
  CATCH_CHECK(M(1,0) == -3.);
  CATCH_CHECK(M(1,1) == -4.);
}

CATCH_TEST_CASE("dynamic unary_minus2")
{
  cml::matrix<double, cml::dynamic<>> M1(
    2,2,
    1., 2.,
    3., 4.
    );
  cml::matrix<double, cml::dynamic<>> M = - M1;
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == -1.);
  CATCH_CHECK(M(0,1) == -2.);
  CATCH_CHECK(M(1,0) == -3.);
  CATCH_CHECK(M(1,1) == -4.);
}

CATCH_TEST_CASE("dynamic unary_plus1")
{
  cml::matrix<double, cml::dynamic<>> M1(
    2,2,
    1., 2.,
    3., 4.
    );
  cml::matrix<double, cml::dynamic<>> M;
  M = + M1;
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(0,1) == 2.);
  CATCH_CHECK(M(1,0) == 3.);
  CATCH_CHECK(M(1,1) == 4.);
}

CATCH_TEST_CASE("dynamic unary_plus2")
{
  cml::matrix<double, cml::dynamic<>> M1(
    2,2,
    1., 2.,
    3., 4.
    );
  cml::matrix<double, cml::dynamic<>> M = + M1;
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(0,1) == 2.);
  CATCH_CHECK(M(1,0) == 3.);
  CATCH_CHECK(M(1,1) == 4.);
}

CATCH_TEST_CASE("dynamic double_negate1")
{
  cml::matrix<double, cml::dynamic<>> M1(
    2,2,
    1., 2.,
    3., 4.
    );
  cml::matrix<double, cml::dynamic<>> M;
  auto xpr = - (-M1);
  M = xpr;
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(0,1) == 2.);
  CATCH_CHECK(M(1,0) == 3.);
  CATCH_CHECK(M(1,1) == 4.);
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
