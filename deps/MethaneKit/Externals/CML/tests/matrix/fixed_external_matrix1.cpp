/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <cml/matrix/fixed_external.h>
#include <cml/matrix/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("typecheck")
{
  CATCH_CHECK((std::is_same<cml::external34d::basis_tag,cml::col_basis>::value));
  CATCH_CHECK((std::is_same<cml::external34d::layout_tag,cml::row_major>::value));
  CATCH_CHECK((std::is_same<cml::external34d_c::basis_tag,cml::col_basis>::value));
  CATCH_CHECK((std::is_same<cml::external34d_c::layout_tag,cml::col_major>::value));
}

CATCH_TEST_CASE("array_construct1")
{
  double data[] = {
    1.,  2.,  3.,  4.,
    5.,  6.,  7.,  8.,
    9.,  0.,  0.,  0.
  };
  cml::external34d M(data);
  // Note: external<> refers to the wrapped data in the layout assigned to
  // the matrix.

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_REQUIRE(M.data() == &data[0]);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("array_construct2")
{
  double data[] = {
    1., 5., 9.,
    2., 6., 0.,
    3., 7., 0.,
    4., 8., 0.
  };
  cml::external34d_c M(data);
  // Note: external<> refers to the wrapped data in the layout assigned to
  // the matrix.

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_REQUIRE(M.data() == &data[0]);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("array_assign1")
{
  double data[3*4];
  double aM[] = {
    1.,  2.,  3.,  4.,
    5.,  6.,  7.,  8.,
    9.,  0.,  0.,  0.
  };
  cml::external34d M(data);
  M = aM;
  // Note: external<> copies arrays using row-major layout.

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_REQUIRE(M.data() == &data[0]);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("array_assign2")
{
  double data[3*4];
  double aM[] = {
    1.,  2.,  3.,  4.,
    5.,  6.,  7.,  8.,
    9.,  0.,  0.,  0.
  };
  cml::external34d_c M(data);
  M = aM;
  // Note: external<> copies arrays using row-major layout.

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_REQUIRE(M.data() == &data[0]);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("array2_construct1")
{
  double data[3][4] = {
    { 1.,  2.,  3.,  4. },
    { 5.,  6.,  7.,  8. },
    { 9.,  0.,  0.,  0. }
  };
  cml::external34d M(data);
  // Note: external<> refers to the wrapped data in the layout assigned to
  // the matrix.

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_REQUIRE(M.data() == &data[0][0]);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("array2_construct2")
{
  double data[4][3] = {
    { 1., 5., 9. },
    { 2., 6., 0. },
    { 3., 7., 0. },
    { 4., 8., 0. }
  };
  cml::external34d_c M(data);
  // Note: external<> refers to the wrapped data in the layout assigned to
  // the matrix.

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_REQUIRE(M.data() == &data[0][0]);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("array2_assign1")
{
  double data[3*4];
  double aM[3][4] = {
    { 1.,  2.,  3.,  4. },
    { 5.,  6.,  7.,  8. },
    { 9.,  0.,  0.,  0. }
  };
  cml::external34d M(data);
  M = aM;
  // Note: external<> copies arrays using row-major layout.

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_REQUIRE(M.data() == &data[0]);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("array2_assign2")
{
  double data[3*4];
  double aM[3][4] = {
    { 1.,  2.,  3.,  4. },
    { 5.,  6.,  7.,  8. },
    { 9.,  0.,  0.,  0. }
  };
  cml::external34d_c M(data);
  M = aM;
  // Note: external<> copies arrays using row-major layout.

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_REQUIRE(M.data() == &data[0]);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("list_assign1")
{
  double data[3*4];
  cml::external34d M(data);
  M = {
    1.,  2.,  3.,  4.,
    5.,  6.,  7.,  8.,
    9.,  0.,  0.,  0.
  };
  // Note: external<> copies arrays using row-major layout.

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_REQUIRE(M.data() == &data[0]);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("list_assign2")
{
  double data[3*4];
  cml::external34d_c M(data);
  M = {
    1.,  2.,  3.,  4.,
    5.,  6.,  7.,  8.,
    9.,  0.,  0.,  0.
  };
  // Note: external<> copies arrays using row-major layout.

  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_REQUIRE(M.data() == &data[0]);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(0,0) == 1.);
  CATCH_CHECK(M(2,0) == 9.);
  CATCH_CHECK(M(2,1) == 0.);
  CATCH_CHECK(M(2,2) == 0.);
  CATCH_CHECK(M(2,3) == 0.);
}

CATCH_TEST_CASE("fill1")
{
  double data[3*4];
  cml::external34d_c M(data);
  M.fill(1.);
  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_CHECK(M.data()[0] == 1.);
  CATCH_CHECK(M(2,3) == 1.);
}

CATCH_TEST_CASE("size_check1")
{
  double data[3*4];
  cml::external34d M(data);
  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_REQUIRE(M.data() == &data[0]);
  CATCH_REQUIRE_THROWS_AS(
    (M = {
     1.,  2.,  3.,  4.,
     5.,  6.,  7.,  8.,
     9.
     }), cml::incompatible_matrix_size_error);
}

CATCH_TEST_CASE("size_check2")
{
  double data[3*4];
  cml::external34d_c M(data);
  CATCH_REQUIRE(M.rows() == 3);
  CATCH_REQUIRE(M.cols() == 4);
  CATCH_REQUIRE(M.data() == &data[0]);
  CATCH_REQUIRE_THROWS_AS(
    (M = {
     1.,  2.,  3.,  4.,
     5.,  6.,  7.,  8.,
     9.
     }), cml::incompatible_matrix_size_error);
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
