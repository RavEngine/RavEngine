/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/vector/binary_node.h>
#include <cml/vector/binary_ops.h>

#include <cml/vector/fixed.h>
#include <cml/vector/external.h>
#include <cml/vector/dynamic.h>
#include <cml/vector/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("binary_types1")
{
  typedef cml::vector3d vector_type;
  {
    CATCH_CHECK(cml::is_statically_polymorphic<vector_type>::value);
  }
  {
    auto xpr = vector_type() + vector_type();
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::right_arg_type>::value
      );
  }
  {
    auto xpr = vector_type() - vector_type();
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::right_arg_type>::value
      );
  }
  {
    vector_type M;
    auto xpr = vector_type() + M;
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::right_arg_type>::value
      );
  }
  {
    vector_type M;
    auto xpr = M + vector_type();
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::right_arg_type>::value
      );
  }
  {
    vector_type M;
    auto xpr = vector_type() - M;
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::right_arg_type>::value
      );
  }
  {
    vector_type M;
    auto xpr = M - vector_type();
    typedef decltype(xpr) xpr_type;
    CATCH_CHECK(
      std::is_lvalue_reference<typename xpr_type::left_arg_type>::value
      );
    CATCH_CHECK(
      std::is_rvalue_reference<typename xpr_type::right_arg_type>::value
      );
  }
  {
    vector_type M1, M2;
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
    vector_type M1, M2;
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
  cml::vector3d v1 = { 1., 2., 3. };
  cml::vector3d v2 = { 4., 5., 6. };
  cml::vector3d w;
  w = v1 - v2;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == -3.);
  CATCH_CHECK(w[1] == -3.);
  CATCH_CHECK(w[2] == -3.);
}

CATCH_TEST_CASE("fixed, binary_minus2")
{
  cml::vector3d v1 = { 1., 2., 3. };
  cml::vector3d v2 = { 4., 5., 6. };
  cml::vector3d w = v1 - v2;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == -3.);
  CATCH_CHECK(w[1] == -3.);
  CATCH_CHECK(w[2] == -3.);
}

CATCH_TEST_CASE("fixed, binary_plus1")
{
  cml::vector3d v1 = { 1., 2., 3. };
  cml::vector3d v2 = { 4., 5., 6. };
  cml::vector3d w;
  w = v1 + v2;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 5.);
  CATCH_CHECK(w[1] == 7.);
  CATCH_CHECK(w[2] == 9.);
}

CATCH_TEST_CASE("fixed, binary_plus2")
{
  cml::vector3d v1 = { 1., 2., 3. };
  cml::vector3d v2 = { 4., 5., 6. };
  cml::vector3d w = v1 + v2;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 5.);
  CATCH_CHECK(w[1] == 7.);
  CATCH_CHECK(w[2] == 9.);
}

CATCH_TEST_CASE("fixed, multiple_plus1")
{
  cml::vector3d v1 = { 1., 2., 3. };
  cml::vector3d v2 = { 4., 5., 6. };
  cml::vector3d v3 = { 7., 8., 9. };
  cml::vector3d w;
  w = v1 + (v2 + v3);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 12.);
  CATCH_CHECK(w[1] == 15.);
  CATCH_CHECK(w[2] == 18.);
}

CATCH_TEST_CASE("fixed, multiple_plus2")
{
  cml::vector3d v1 = { 1., 2., 3. };
  cml::vector3d v2 = { 4., 5., 6. };
  cml::vector3d v3 = { 7., 8., 9. };
  cml::vector3d w = v1 + (v2 + v3);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 12.);
  CATCH_CHECK(w[1] == 15.);
  CATCH_CHECK(w[2] == 18.);
}

CATCH_TEST_CASE("fixed, mixed_op1")
{
  cml::vector3d v1 = { 1., 2., 3. };
  cml::vector3d v2 = { 4., 5., 6. };
  cml::vector3d v3 = { 7., 8., 9. };
  cml::vector3d w;
  w = v2 - (v1 + v3);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == -4.);
  CATCH_CHECK(w[1] == -5.);
  CATCH_CHECK(w[2] == -6.);
}

CATCH_TEST_CASE("fixed, mixed_op2")
{
  cml::vector3d v1 = { 1., 2., 3. };
  cml::vector3d v2 = { 4., 5., 6. };
  cml::vector3d v3 = { 7., 8., 9. };
  cml::vector3d w = v2 - (v1 + v3);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == -4.);
  CATCH_CHECK(w[1] == -5.);
  CATCH_CHECK(w[2] == -6.);
}

CATCH_TEST_CASE("fixed, assign_minus1")
{
  cml::vector3d w = { 1., 2., 3. };
  cml::vector3d v = { 4., 5., 6. };
  w -= v;
  CATCH_CHECK(w[0] == -3.);
  CATCH_CHECK(w[1] == -3.);
  CATCH_CHECK(w[2] == -3.);
}

CATCH_TEST_CASE("fixed, assign_plus1")
{
  cml::vector3d w = { 1., 2., 3. };
  cml::vector3d v = { 4., 5., 6. };
  w += v;
  CATCH_CHECK(w[0] == 5.);
  CATCH_CHECK(w[1] == 7.);
  CATCH_CHECK(w[2] == 9.);
}




CATCH_TEST_CASE("fixed external, binary_minus1")
{
  double av1[] = { 1., 2., 3. };
  double av2[] = { 4., 5., 6. };
  double aw[3];
  cml::external3d v1(av1);
  cml::external3d v2(av2);
  cml::external3d w(aw);
  w = v1 - v2;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == -3.);
  CATCH_CHECK(w[1] == -3.);
  CATCH_CHECK(w[2] == -3.);
}

CATCH_TEST_CASE("fixed external, binary_plus1")
{
  double av1[] = { 1., 2., 3. };
  double av2[] = { 4., 5., 6. };
  double aw[3];
  cml::external3d v1(av1);
  cml::external3d v2(av2);
  cml::external3d w(aw);
  w = v1 + v2;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 5.);
  CATCH_CHECK(w[1] == 7.);
  CATCH_CHECK(w[2] == 9.);
}

CATCH_TEST_CASE("fixed external, multiple_plus1")
{
  double av1[] = { 1., 2., 3. };
  double av2[] = { 4., 5., 6. };
  double av3[] = { 7., 8., 9. };
  double aw[3];
  cml::external3d v1(av1);
  cml::external3d v2(av2);
  cml::external3d v3(av3);
  cml::external3d w(aw);
  w = v1 + (v2 + v3);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 12.);
  CATCH_CHECK(w[1] == 15.);
  CATCH_CHECK(w[2] == 18.);
}

CATCH_TEST_CASE("fixed external, mixed_op1")
{
  double av1[] = { 1., 2., 3. };
  double av2[] = { 4., 5., 6. };
  double av3[] = { 7., 8., 9. };
  double aw[3];
  cml::external3d v1(av1);
  cml::external3d v2(av2);
  cml::external3d v3(av3);
  cml::external3d w(aw);
  w = v2 - (v1 + v3);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == -4.);
  CATCH_CHECK(w[1] == -5.);
  CATCH_CHECK(w[2] == -6.);
}




CATCH_TEST_CASE("dynamic external, binary_minus1")
{
  double av1[] = { 1., 2., 3. };
  double av2[] = { 4., 5., 6. };
  double aw[3];
  cml::externalnd v1(av1,3);
  cml::externalnd v2(av2,3);
  cml::externalnd w(aw,3);
  w = v1 - v2;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == -3.);
  CATCH_CHECK(w[1] == -3.);
  CATCH_CHECK(w[2] == -3.);
}

CATCH_TEST_CASE("dynamic external, binary_plus1")
{
  double av1[] = { 1., 2., 3. };
  double av2[] = { 4., 5., 6. };
  double aw[3];
  cml::externalnd v1(av1,3);
  cml::externalnd v2(av2,3);
  cml::externalnd w(aw,3);
  w = v1 + v2;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 5.);
  CATCH_CHECK(w[1] == 7.);
  CATCH_CHECK(w[2] == 9.);
}

CATCH_TEST_CASE("dynamic external, multiple_plus1")
{
  double av1[] = { 1., 2., 3. };
  double av2[] = { 4., 5., 6. };
  double av3[] = { 7., 8., 9. };
  double aw[3];
  cml::externalnd v1(av1,3);
  cml::externalnd v2(av2,3);
  cml::externalnd v3(av3,3);
  cml::externalnd w(aw,3);
  w = v1 + (v2 + v3);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 12.);
  CATCH_CHECK(w[1] == 15.);
  CATCH_CHECK(w[2] == 18.);
}

CATCH_TEST_CASE("dynamic external, mixed_op1")
{
  double av1[] = { 1., 2., 3. };
  double av2[] = { 4., 5., 6. };
  double av3[] = { 7., 8., 9. };
  double aw[3];
  cml::externalnd v1(av1,3);
  cml::externalnd v2(av2,3);
  cml::externalnd v3(av3,3);
  cml::externalnd w(aw,3);
  w = v2 - (v1 + v3);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == -4.);
  CATCH_CHECK(w[1] == -5.);
  CATCH_CHECK(w[2] == -6.);
}




CATCH_TEST_CASE("dynamic const external, binary_minus1")
{
  const double av1[] = { 1., 2., 3. };
  const double av2[] = { 4., 5., 6. };
  double aw[3];
  cml::externalncd v1(av1,3);
  cml::externalncd v2(av2,3);
  cml::externalnd w(aw,3);
  w = v1 - v2;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == -3.);
  CATCH_CHECK(w[1] == -3.);
  CATCH_CHECK(w[2] == -3.);
}

CATCH_TEST_CASE("dynamic const external, binary_plus1")
{
  const double av1[] = { 1., 2., 3. };
  const double av2[] = { 4., 5., 6. };
  double aw[3];
  cml::externalncd v1(av1,3);
  cml::externalncd v2(av2,3);
  cml::externalnd w(aw,3);
  w = v1 + v2;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 5.);
  CATCH_CHECK(w[1] == 7.);
  CATCH_CHECK(w[2] == 9.);
}

CATCH_TEST_CASE("dynamic const external, multiple_plus1")
{
  const double av1[] = { 1., 2., 3. };
  const double av2[] = { 4., 5., 6. };
  double av3[] = { 7., 8., 9. };
  double aw[3];
  cml::externalncd v1(av1,3);
  cml::externalncd v2(av2,3);
  cml::externalnd v3(av3,3);
  cml::externalnd w(aw,3);
  w = v1 + (v2 + v3);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 12.);
  CATCH_CHECK(w[1] == 15.);
  CATCH_CHECK(w[2] == 18.);
}

CATCH_TEST_CASE("dynamic const external, mixed_op1")
{
  const double av1[] = { 1., 2., 3. };
  double av2[] = { 4., 5., 6. };
  const double av3[] = { 7., 8., 9. };
  double aw[3];
  cml::externalncd v1(av1,3);
  cml::externalnd v2(av2,3);
  cml::externalncd v3(av3,3);
  cml::externalnd w(aw,3);
  w = v2 - (v1 + v3);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == -4.);
  CATCH_CHECK(w[1] == -5.);
  CATCH_CHECK(w[2] == -6.);
}




CATCH_TEST_CASE("dynamic, binary_minus1")
{
  cml::vectord v1 = { 1., 2., 3. };
  cml::vectord v2 = { 4., 5., 6. };
  cml::vectord w;
  w = v1 - v2;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == -3.);
  CATCH_CHECK(w[1] == -3.);
  CATCH_CHECK(w[2] == -3.);
}

CATCH_TEST_CASE("dynamic, binary_minus2")
{
  cml::vectord v1 = { 1., 2., 3. };
  cml::vectord v2 = { 4., 5., 6. };
  cml::vectord w = v1 - v2;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == -3.);
  CATCH_CHECK(w[1] == -3.);
  CATCH_CHECK(w[2] == -3.);
}

CATCH_TEST_CASE("dynamic, binary_plus1")
{
  cml::vectord v1 = { 1., 2., 3. };
  cml::vectord v2 = { 4., 5., 6. };
  cml::vectord w;
  w = v1 + v2;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 5.);
  CATCH_CHECK(w[1] == 7.);
  CATCH_CHECK(w[2] == 9.);
}

CATCH_TEST_CASE("dynamic, binary_plus2")
{
  cml::vectord v1 = { 1., 2., 3. };
  cml::vectord v2 = { 4., 5., 6. };
  cml::vectord w = v1 + v2;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 5.);
  CATCH_CHECK(w[1] == 7.);
  CATCH_CHECK(w[2] == 9.);
}

CATCH_TEST_CASE("dynamic, multiple_plus1")
{
  cml::vectord v1 = { 1., 2., 3. };
  cml::vectord v2 = { 4., 5., 6. };
  cml::vectord v3 = { 7., 8., 9. };
  cml::vectord w;
  w = v1 + (v2 + v3);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 12.);
  CATCH_CHECK(w[1] == 15.);
  CATCH_CHECK(w[2] == 18.);
}

CATCH_TEST_CASE("dynamic, multiple_plus2")
{
  cml::vectord v1 = { 1., 2., 3. };
  cml::vectord v2 = { 4., 5., 6. };
  cml::vectord v3 = { 7., 8., 9. };
  cml::vectord w = v1 + (v2 + v3);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 12.);
  CATCH_CHECK(w[1] == 15.);
  CATCH_CHECK(w[2] == 18.);
}

CATCH_TEST_CASE("dynamic, mixed_op1")
{
  cml::vectord v1 = { 1., 2., 3. };
  cml::vectord v2 = { 4., 5., 6. };
  cml::vectord v3 = { 7., 8., 9. };
  cml::vectord w;
  w = v2 - (v1 + v3);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == -4.);
  CATCH_CHECK(w[1] == -5.);
  CATCH_CHECK(w[2] == -6.);
}

CATCH_TEST_CASE("dynamic, mixed_op2")
{
  cml::vectord v1 = { 1., 2., 3. };
  cml::vectord v2 = { 4., 5., 6. };
  cml::vectord v3 = { 7., 8., 9. };
  cml::vectord w = v2 - (v1 + v3);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == -4.);
  CATCH_CHECK(w[1] == -5.);
  CATCH_CHECK(w[2] == -6.);
}

CATCH_TEST_CASE("dynamic, assign_minus1")
{
  cml::vectord w = { 1., 2., 3. };
  cml::vectord v = { 4., 5., 6. };
  w -= v;
  CATCH_CHECK(w[0] == -3.);
  CATCH_CHECK(w[1] == -3.);
  CATCH_CHECK(w[2] == -3.);
}

CATCH_TEST_CASE("dynamic, assign_plus1")
{
  cml::vectord w = { 1., 2., 3. };
  cml::vectord v = { 4., 5., 6. };
  w += v;
  CATCH_CHECK(w[0] == 5.);
  CATCH_CHECK(w[1] == 7.);
  CATCH_CHECK(w[2] == 9.);
}




CATCH_TEST_CASE("mixed fixed storage, construct_xpr")
{
  cml::vector3d v1 = { 1., 2., 3. };
  double av2[] = {  7.,  8.,  9. };
  cml::external3d v2(av2);

  cml::vector3d v3 = { 4., 5., 6. };
  double av4[] = { 10., 11., 12. };
  cml::external3d v4(av4);

  cml::vector3d w = v1 + v2 - v3 + v4;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 14.);
  CATCH_CHECK(w[1] == 16.);
  CATCH_CHECK(w[2] == 18.);
}

CATCH_TEST_CASE("mixed fixed storage, assign_xpr")
{
  cml::vector3d v1 = { 1., 2., 3. };
  double av2[] = {  7.,  8.,  9. };
  cml::external3d v2(av2);

  cml::vector3d v3 = { 4., 5., 6. };
  double av4[] = { 10., 11., 12. };
  cml::external3d v4(av4);

  cml::vector3d w;
  w = v1 + v2 - v3 + v4;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 14.);
  CATCH_CHECK(w[1] == 16.);
  CATCH_CHECK(w[2] == 18.);
}

CATCH_TEST_CASE("mixed fixed storage, assign_temp_xpr")
{
  cml::vector3d v1 = { 1., 2., 3. };
  double av2[] = {  7.,  8.,  9. };
  cml::external3d v2(av2);

  cml::vector3d v3 = { 4., 5., 6. };
  double av4[] = { 10., 11., 12. };
  cml::external3d v4(av4);

  auto xpr = v1 + v2 - v3 + v4;
  // CATCH_CHECK(sizeof(xpr) == 32U);

  cml::vector3d w;
  w = xpr;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 14.);
  CATCH_CHECK(w[1] == 16.);
  CATCH_CHECK(w[2] == 18.);
}




CATCH_TEST_CASE("mixed storage, construct_xpr")
{
  cml::vector3d v1 = { 1., 2., 3. };
  double av2[] = {  7.,  8.,  9. };
  cml::external3d v2(av2);

  cml::vectord v3 = { 4., 5., 6. };
  double av4[] = { 10., 11., 12. };
  cml::externalnd v4(av4, 3);

  cml::vector3d w = v1 + v2 - v3 + v4;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 14.);
  CATCH_CHECK(w[1] == 16.);
  CATCH_CHECK(w[2] == 18.);
}

CATCH_TEST_CASE("mixed storage, assign_xpr")
{
  cml::vector3d v1 = { 1., 2., 3. };
  double av2[] = {  7.,  8.,  9. };
  cml::external3d v2(av2);

  cml::vectord v3 = { 4., 5., 6. };
  double av4[] = { 10., 11., 12. };
  cml::externalnd v4(av4, 3);

  cml::vectord w;
  w = v1 + v2 - v3 + v4;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 14.);
  CATCH_CHECK(w[1] == 16.);
  CATCH_CHECK(w[2] == 18.);
}

CATCH_TEST_CASE("mixed storage, assign_temp_xpr")
{
  cml::vector3d v1 = { 1., 2., 3. };
  double av2[] = {  7.,  8.,  9. };
  cml::external3d v2(av2);

  cml::vectord v3 = { 4., 5., 6. };
  double av4[] = { 10., 11., 12. };
  cml::externalnd v4(av4, 3);

  auto xpr = v1 + v2 - v3 + v4;
  // CATCH_CHECK(sizeof(xpr) == 32U);

  double aw[3];
  cml::externalnd w(aw, 3);
  w = xpr;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 14.);
  CATCH_CHECK(w[1] == 16.);
  CATCH_CHECK(w[2] == 18.);
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
