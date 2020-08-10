/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/vector/scalar_node.h>
#include <cml/vector/scalar_ops.h>

#include <cml/vector/fixed.h>
#include <cml/vector/external.h>
#include <cml/vector/dynamic.h>
#include <cml/vector/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("scalar_types1")
{
  typedef cml::vector3d vector_type;
  {
    auto xpr = vector_type()*int();
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
    auto xpr = int()*vector_type();
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
    auto xpr = vector_type()/int();
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
    auto xpr = vector_type()*v;
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
    auto xpr = v*vector_type();
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
    auto xpr = vector_type()/v;
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
  cml::vector3d v1 = { 1., 2., 3. };
  cml::vector3d w;
  w = 2.*v1;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 2.);
  CATCH_CHECK(w[1] == 4.);
  CATCH_CHECK(w[2] == 6.);
}

CATCH_TEST_CASE("fixed, scalar_multiply2")
{
  cml::vector3d v1 = { 1., 2., 3. };
  cml::vector3d w = 2.*v1;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 2.);
  CATCH_CHECK(w[1] == 4.);
  CATCH_CHECK(w[2] == 6.);
}

CATCH_TEST_CASE("fixed, scalar_divide1")
{
  cml::vector3d v1 = { 2., 4., 6. };
  cml::vector3d w;
  w = v1/2.;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 1.);
  CATCH_CHECK(w[1] == 2.);
  CATCH_CHECK(w[2] == 3.);
}

CATCH_TEST_CASE("fixed, scalar_divide2")
{
  cml::vector3d v1 = { 2., 4., 6. };
  cml::vector3d w = v1/2.;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 1.);
  CATCH_CHECK(w[1] == 2.);
  CATCH_CHECK(w[2] == 3.);
}

CATCH_TEST_CASE("fixed, scalar_multiply_assign1")
{
  cml::vector3d v1 = { 1., 2., 3. };
  v1 *= 2;
  CATCH_REQUIRE(v1.size() == 3);
  CATCH_CHECK(v1[0] == 2.);
  CATCH_CHECK(v1[1] == 4.);
  CATCH_CHECK(v1[2] == 6.);
}

CATCH_TEST_CASE("fixed, scalar_multiply_assign2")
{
  cml::vector3d w;
  w = (cml::vector3d(1.,2.,3.) *= 2);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 2.);
  CATCH_CHECK(w[1] == 4.);
  CATCH_CHECK(w[2] == 6.);
}

CATCH_TEST_CASE("fixed, scalar_divide_assign1")
{
  cml::vector3d v1 = { 2., 4., 6. };
  v1 /= 2;
  CATCH_REQUIRE(v1.size() == 3);
  CATCH_CHECK(v1[0] == 1.);
  CATCH_CHECK(v1[1] == 2.);
  CATCH_CHECK(v1[2] == 3.);
}

CATCH_TEST_CASE("fixed, scalar_divide_assign2")
{
  cml::vector3d w;
  w = (cml::vector3d(2.,4.,6.) /= 2);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 1.);
  CATCH_CHECK(w[1] == 2.);
  CATCH_CHECK(w[2] == 3.);
}




CATCH_TEST_CASE("fixed external, scalar_multiply1")
{
  double av1[] = { 1., 2., 3. };
  double aw[3];
  cml::external3d v1(av1);
  cml::external3d w(aw);
  w = 2.*v1;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 2.);
  CATCH_CHECK(w[1] == 4.);
  CATCH_CHECK(w[2] == 6.);
}

CATCH_TEST_CASE("fixed external, scalar_divide1")
{
  double av1[] = { 2., 4., 6. };
  double aw[3];
  cml::external3d v1(av1);
  cml::external3d w(aw);
  w = v1/2.;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 1.);
  CATCH_CHECK(w[1] == 2.);
  CATCH_CHECK(w[2] == 3.);
}

CATCH_TEST_CASE("fixed external, scalar_multiply_assign1")
{
  double av1[] = { 1., 2., 3. };
  cml::external3d v1(av1);
  v1 *= 2;
  CATCH_REQUIRE(v1.size() == 3);
  CATCH_CHECK(v1[0] == 2.);
  CATCH_CHECK(v1[1] == 4.);
  CATCH_CHECK(v1[2] == 6.);
}

CATCH_TEST_CASE("fixed external, scalar_multiply_assign2")
{
#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
  double av1[] = { 1., 2., 3. };
  cml::external3d w;
  w = (cml::external3d(av1) *= 2);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w.data() == &av1[0]);
  CATCH_CHECK(w[0] == 2.);
  CATCH_CHECK(w[1] == 4.);
  CATCH_CHECK(w[2] == 6.);
#else
  CATCH_WARN("Assignment to temporary external vectors not supported");
#endif
}

CATCH_TEST_CASE("fixed external, scalar_divide_assign1")
{
  double av1[] = { 2., 4., 6. };
  cml::external3d v1(av1);
  v1 /= 2;
  CATCH_REQUIRE(v1.size() == 3);
  CATCH_CHECK(v1[0] == 1.);
  CATCH_CHECK(v1[1] == 2.);
  CATCH_CHECK(v1[2] == 3.);
}

CATCH_TEST_CASE("fixed external, scalar_divide_assign2")
{
#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
  double av1[] = { 2., 4., 6. };
  cml::external3d w;
  w = (cml::external3d(av1) /= 2);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w.data() == &av1[0]);
  CATCH_CHECK(w[0] == 1.);
  CATCH_CHECK(w[1] == 2.);
  CATCH_CHECK(w[2] == 3.);
#else
  CATCH_WARN("Assignment to temporary external vectors not supported");
#endif
}




CATCH_TEST_CASE("dynamic external, scalar_multiply1")
{
  double av1[] = { 1., 2., 3. };
  double aw[3];
  cml::externalnd v1(av1,3);
  cml::externalnd w(aw,3);
  w = 2.*v1;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 2.);
  CATCH_CHECK(w[1] == 4.);
  CATCH_CHECK(w[2] == 6.);
}

CATCH_TEST_CASE("dynamic external, scalar_divide1")
{
  double av1[] = { 2., 4., 6. };
  double aw[3];
  cml::externalnd v1(av1,3);
  cml::externalnd w(aw,3);
  w = v1/2.;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 1.);
  CATCH_CHECK(w[1] == 2.);
  CATCH_CHECK(w[2] == 3.);
}

CATCH_TEST_CASE("dynamic external, scalar_multiply_assign1")
{
  double av1[] = { 1., 2., 3. };
  cml::externalnd v1(av1,3);
  v1 *= 2;
  CATCH_REQUIRE(v1.size() == 3);
  CATCH_CHECK(v1[0] == 2.);
  CATCH_CHECK(v1[1] == 4.);
  CATCH_CHECK(v1[2] == 6.);
}

CATCH_TEST_CASE("dynamic external, scalar_multiply_assign2")
{
#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
  double av1[] = { 1., 2., 3. };
  cml::externalnd w;
  w = (cml::externalnd(av1,3) *= 2);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w.data() == &av1[0]);
  CATCH_CHECK(w[0] == 2.);
  CATCH_CHECK(w[1] == 4.);
  CATCH_CHECK(w[2] == 6.);
#else
  CATCH_WARN("Assignment to temporary external vectors not supported");
#endif
}

CATCH_TEST_CASE("dynamic external, scalar_divide_assign1")
{
  double av1[] = { 2., 4., 6. };
  cml::externalnd v1(av1,3);
  v1 /= 2;
  CATCH_REQUIRE(v1.size() == 3);
  CATCH_CHECK(v1[0] == 1.);
  CATCH_CHECK(v1[1] == 2.);
  CATCH_CHECK(v1[2] == 3.);
}

CATCH_TEST_CASE("dynamic external, scalar_divide_assign2")
{
#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
  double av1[] = { 2., 4., 6. };
  cml::externalnd w;
  w = (cml::externalnd(av1,3) /= 2);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w.data() == &av1[0]);
  CATCH_CHECK(w[0] == 1.);
  CATCH_CHECK(w[1] == 2.);
  CATCH_CHECK(w[2] == 3.);
#else
  CATCH_WARN("Assignment to temporary external vectors not supported");
#endif
}


CATCH_TEST_CASE("dynamic, scalar_multiply1")
{
  cml::vectord v1 = { 1., 2., 3. };
  cml::vectord w;
  w = 2.*v1;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 2.);
  CATCH_CHECK(w[1] == 4.);
  CATCH_CHECK(w[2] == 6.);
}

CATCH_TEST_CASE("dynamic, scalar_multiply2")
{
  cml::vectord v1 = { 1., 2., 3. };
  cml::vectord w = 2.*v1;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 2.);
  CATCH_CHECK(w[1] == 4.);
  CATCH_CHECK(w[2] == 6.);
}

CATCH_TEST_CASE("dynamic, scalar_divide1")
{
  cml::vectord v1 = { 2., 4., 6. };
  cml::vectord w;
  w = v1/2.;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 1.);
  CATCH_CHECK(w[1] == 2.);
  CATCH_CHECK(w[2] == 3.);
}

CATCH_TEST_CASE("dynamic, scalar_divide2")
{
  cml::vectord v1 = { 2., 4., 6. };
  cml::vectord w = v1/2.;
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 1.);
  CATCH_CHECK(w[1] == 2.);
  CATCH_CHECK(w[2] == 3.);
}

CATCH_TEST_CASE("dynamic, scalar_multiply_assign1")
{
  cml::vectord v1 = { 1., 2., 3. };
  v1 *= 2;
  CATCH_REQUIRE(v1.size() == 3);
  CATCH_CHECK(v1[0] == 2.);
  CATCH_CHECK(v1[1] == 4.);
  CATCH_CHECK(v1[2] == 6.);
}

CATCH_TEST_CASE("dynamic, scalar_multiply_assign2")
{
  cml::vectord w;
  w = (cml::vector<double, cml::dynamic<>>(1.,2.,3.) *= 2);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 2.);
  CATCH_CHECK(w[1] == 4.);
  CATCH_CHECK(w[2] == 6.);
}

CATCH_TEST_CASE("dynamic, scalar_divide_assign1")
{
  cml::vectord v1 = { 2., 4., 6. };
  v1 /= 2;
  CATCH_REQUIRE(v1.size() == 3);
  CATCH_CHECK(v1[0] == 1.);
  CATCH_CHECK(v1[1] == 2.);
  CATCH_CHECK(v1[2] == 3.);
}

CATCH_TEST_CASE("dynamic, scalar_divide_assign2")
{
  cml::vectord w;
  w = (cml::vectord(2.,4.,6.) /= 2);
  CATCH_REQUIRE(w.size() == 3);
  CATCH_CHECK(w[0] == 1.);
  CATCH_CHECK(w[1] == 2.);
  CATCH_CHECK(w[2] == 3.);
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
