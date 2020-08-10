/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <cml/matrix/dynamic.h>
#include <cml/matrix/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("typecheck")
{
  CATCH_CHECK((std::is_same<cml::matrixd::basis_tag,cml::col_basis>::value));
  CATCH_CHECK((std::is_same<cml::matrixd::layout_tag,cml::row_major>::value));
  CATCH_CHECK((std::is_same<cml::matrixd_c::basis_tag,cml::col_basis>::value));
  CATCH_CHECK((std::is_same<cml::matrixd_c::layout_tag,cml::col_major>::value));
}

CATCH_TEST_CASE("alloc1")
{
  cml::matrixd M(3,4);
  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
}

CATCH_TEST_CASE("alloc2")
{
  cml::matrixd_c M(3,4);
  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
}

CATCH_TEST_CASE("resize1")
{
  cml::matrixd M(2,2);
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  M.resize(3,4);
  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
}

CATCH_TEST_CASE("resize2")
{
  cml::matrixd_c M(2,2);
  CATCH_REQUIRE(M.rows() == 2);
  CATCH_REQUIRE(M.cols() == 2);
  M.resize(3,4);
  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
}

CATCH_TEST_CASE("array_construct1")
{
  double aM[] = {
    1.,  2.,  3.,  4.,
    5.,  6.,  7.,  8.,
    9.,  0.,  0.,  0.
  };
  cml::matrixd M(3,4, aM);

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("array_construct2")
{
  double aM[] = {
    1.,  2.,  3.,  4.,
    5.,  6.,  7.,  8.,
    9.,  0.,  0.,  0.
  };
  cml::matrixd_c M(3,4, aM);

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("array_assign1")
{
  double aM[] = {
    1.,  2.,  3.,  4.,
    5.,  6.,  7.,  8.,
    9.,  0.,  0.,  0.
  };
  cml::matrixd M(3,4);
  M = aM;

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("array_assign2")
{
  double aM[] = {
    1.,  2.,  3.,  4.,
    5.,  6.,  7.,  8.,
    9.,  0.,  0.,  0.
  };
  cml::matrixd_c M(3,4);
  M = aM;

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("array2_construct1")
{
  double aM[3][4] = {
    { 1.,  2.,  3.,  4. },
    { 5.,  6.,  7.,  8. },
    { 9.,  0.,  0.,  0. }
  };
  cml::matrixd M(aM);

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("array2_construct2")
{
  double aM[3][4] = {
    { 1.,  2.,  3.,  4. },
    { 5.,  6.,  7.,  8. },
    { 9.,  0.,  0.,  0. }
  };
  cml::matrixd_c M(aM);

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("array2_temp_construct1")
{
  double aM[3][4] = {
    { 1.,  2.,  3.,  4. },
    { 5.,  6.,  7.,  8. },
    { 9.,  0.,  0.,  0. }
  };
  cml::matrixd M = aM;

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("array2_temp_construct2")
{
  double aM[3][4] = {
    { 1.,  2.,  3.,  4. },
    { 5.,  6.,  7.,  8. },
    { 9.,  0.,  0.,  0. }
  };
  cml::matrixd_c M = aM;

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("array2_assign1")
{
  double aM[3][4] = {
    { 1.,  2.,  3.,  4. },
    { 5.,  6.,  7.,  8. },
    { 9.,  0.,  0.,  0. }
  };
  cml::matrixd M;
  M = aM;

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("array2_assign2")
{
  double aM[3][4] = {
    { 1.,  2.,  3.,  4. },
    { 5.,  6.,  7.,  8. },
    { 9.,  0.,  0.,  0. }
  };
  cml::matrixd_c M;
  M = aM;

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("element_construct1")
{
  cml::matrixd M(
    3, 4,
    1.,  2.,  3.,  4.,
    5.,  6.,  7.,  8.,
    9.,  0.,  0.,  0.
    );

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("element_construct2")
{
  cml::matrixd_c M(
    3, 4,
    1.,  2.,  3.,  4.,
    5.,  6.,  7.,  8.,
    9.,  0.,  0.,  0.
    );

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("pointer_construct1")
{
  double aM[3][4] = {
    { 1.,  2.,  3.,  4. },
    { 5.,  6.,  7.,  8. },
    { 9.,  0.,  0.,  0. }
  };
  cml::matrixd M(3,4, &aM[0][0]);

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("pointer_construct2")
{
  double aM[3][4] = {
    { 1.,  2.,  3.,  4. },
    { 5.,  6.,  7.,  8. },
    { 9.,  0.,  0.,  0. }
  };
  cml::matrixd_c M(&aM[0][0], 3,4);

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("list_assign1")
{
  cml::matrixd M(3,4);
  M = {
    1.,  2.,  3.,  4.,
    5.,  6.,  7.,  8.,
    9.,  0.,  0.,  0.
  };

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("list_assign2")
{
  cml::matrixd_c M(3,4);
  M = {
    1.,  2.,  3.,  4.,
    5.,  6.,  7.,  8.,
    9.,  0.,  0.,  0.
  };

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("fill1")
{
  cml::matrixd M(5,5);
  M.fill(1.);
  CATCH_REQUIRE(M.rows() == 5);
  CATCH_REQUIRE(M.cols() == 5);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(4,4) == 1.);
}

CATCH_TEST_CASE("size_check1")
{
  cml::matrixd M(3,4);
  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_REQUIRE_THROWS_AS(
    (M = {
     1.,  2.,  3.,  4.,
     5.,  6.,  7.,  8.,
     9.
     }), cml::incompatible_matrix_size_error);
}

CATCH_TEST_CASE("size_check2")
{
  cml::matrixd_c M(3,4);
  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_REQUIRE_THROWS_AS(
    (M = {
     1.,  2.,  3.,  4.,
     5.,  6.,  7.,  8.,
     9.
     }), cml::incompatible_matrix_size_error);
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
