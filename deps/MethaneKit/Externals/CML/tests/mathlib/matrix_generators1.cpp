/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/mathlib/matrix/generators.h>

#include <cml/vector.h>
#include <cml/matrix.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("zero, zero_2x2_1")
{
  auto M = cml::zero_2x2();
  typedef decltype(M) matrix_type;
  CATCH_CHECK((std::is_same<double, matrix_type::value_type>::value));
  CATCH_CHECK(M.rows() == 2);
  CATCH_CHECK(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 0.);
  CATCH_CHECK(M(0,1) == 0.);
  CATCH_CHECK(M(1,0) == 0.);
  CATCH_CHECK(M(1,1) == 0.);
}

CATCH_TEST_CASE("zero, zero_3x3_1")
{
  auto M = cml::zero_3x3();
  typedef decltype(M) matrix_type;
  CATCH_CHECK((std::is_same<double, matrix_type::value_type>::value));
  CATCH_CHECK(M.rows() == 3);
  CATCH_CHECK(M.cols() == 3);
  CATCH_CHECK(M(0,0) == 0.);
  CATCH_CHECK(M(0,1) == 0.);
  CATCH_CHECK(M(0,2) == 0.);
  CATCH_CHECK(M(1,0) == 0.);
  CATCH_CHECK(M(1,1) == 0.);
  CATCH_CHECK(M(1,2) == 0.);
  CATCH_CHECK(M(2,0) == 0.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
}

CATCH_TEST_CASE("zero, zero_4x4_1")
{
  auto M = cml::zero_4x4();
  typedef decltype(M) matrix_type;
  CATCH_CHECK((std::is_same<double, matrix_type::value_type>::value));
  CATCH_CHECK(M.rows() == 4);
  CATCH_CHECK(M.cols() == 4);
  CATCH_CHECK(M(0,0) == 0.);
  CATCH_CHECK(M(0,1) == 0.);
  CATCH_CHECK(M(0,2) == 0.);
  CATCH_CHECK(M(0,3) == 0.);
  CATCH_CHECK(M(1,0) == 0.);
  CATCH_CHECK(M(1,1) == 0.);
  CATCH_CHECK(M(1,2) == 0.);
  CATCH_CHECK(M(1,3) == 0.);
  CATCH_CHECK(M(2,0) == 0.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
  CATCH_CHECK(M(3,0) == 0.);
  CATCH_CHECK(M(3,1) == 0.);
  CATCH_CHECK(M(3,2) == 0.);
  CATCH_CHECK(M(3,3) == 0.);
}




CATCH_TEST_CASE("zero, identity_2x2_1")
{
  auto M = cml::identity_2x2();
  typedef decltype(M) matrix_type;
  CATCH_CHECK((std::is_same<double, matrix_type::value_type>::value));
  CATCH_CHECK(M.rows() == 2);
  CATCH_CHECK(M.cols() == 2);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(0,1) == 0.);
  CATCH_CHECK(M(1,0) == 0.);
  CATCH_CHECK(M(1,1) == 1.);
}

CATCH_TEST_CASE("zero, identity_3x3_1")
{
  auto M = cml::identity_3x3();
  typedef decltype(M) matrix_type;
  CATCH_CHECK((std::is_same<double, matrix_type::value_type>::value));
  CATCH_CHECK(M.rows() == 3);
  CATCH_CHECK(M.cols() == 3);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(0,1) == 0.);
  CATCH_CHECK(M(0,2) == 0.);
  CATCH_CHECK(M(1,0) == 0.);
  CATCH_CHECK(M(1,1) == 1.);
  CATCH_CHECK(M(1,2) == 0.);
  CATCH_CHECK(M(2,0) == 0.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 1.);
}

CATCH_TEST_CASE("zero, identity_4x4_1")
{
  auto M = cml::identity_4x4();
  typedef decltype(M) matrix_type;
  CATCH_CHECK((std::is_same<double, matrix_type::value_type>::value));
  CATCH_CHECK(M.rows() == 4);
  CATCH_CHECK(M.cols() == 4);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(0,1) == 0.);
  CATCH_CHECK(M(0,2) == 0.);
  CATCH_CHECK(M(0,3) == 0.);
  CATCH_CHECK(M(1,0) == 0.);
  CATCH_CHECK(M(1,1) == 1.);
  CATCH_CHECK(M(1,2) == 0.);
  CATCH_CHECK(M(1,3) == 0.);
  CATCH_CHECK(M(2,0) == 0.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 1.);
  CATCH_CHECK(M(2,3) == 0.);
  CATCH_CHECK(M(3,0) == 0.);
  CATCH_CHECK(M(3,1) == 0.);
  CATCH_CHECK(M(3,2) == 0.);
  CATCH_CHECK(M(3,3) == 1.);
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
