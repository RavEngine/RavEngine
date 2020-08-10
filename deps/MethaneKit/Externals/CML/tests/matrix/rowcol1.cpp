/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/matrix/row_col.h>

#include <cml/vector.h>
#include <cml/matrix.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("fixed row1")
{
  auto M = cml::matrix44d(
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
    );

  auto row1 = cml::row(M, 1);
  CATCH_CHECK(row1.size() == 4);
  for(int j = 0; j < row1.size(); ++ j)
    CATCH_CHECK(row1[j] == M(1,j));
}

CATCH_TEST_CASE("fixed col1")
{
  auto M = cml::matrix44d(
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
    );

  auto col3 = cml::col(M, 3);
  CATCH_CHECK(col3.size() == 4);
  for(int i = 0; i < col3.size(); ++ i)
    CATCH_CHECK(col3[i] == M(i,3));
}

CATCH_TEST_CASE("fixed external row1")
{
  double avM[16] = {
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
  };
  auto M = cml::external44d(avM);

  auto row1 = cml::row(M, 1);
  CATCH_CHECK(row1.size() == 4);
  for(int j = 0; j < row1.size(); ++ j)
    CATCH_CHECK(row1[j] == M(1,j));
}

CATCH_TEST_CASE("fixed external col1")
{
  double avM[16] = {
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
  };
  auto M = cml::external44d(avM);

  auto col3 = cml::col(M, 3);
  CATCH_CHECK(col3.size() == 4);
  for(int i = 0; i < col3.size(); ++ i)
    CATCH_CHECK(col3[i] == M(i,3));
}

CATCH_TEST_CASE("dynamic external row1")
{
  double avM[16] = {
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
  };
  auto M = cml::externalmnd(4,4, avM);

  auto row1 = cml::row(M, 1);
  CATCH_CHECK(row1.size() == 4);
  for(int j = 0; j < row1.size(); ++ j)
    CATCH_CHECK(row1[j] == M(1,j));
}

CATCH_TEST_CASE("dynamic external col1")
{
  double avM[16] = {
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
  };
  auto M = cml::externalmnd(4,4, avM);

  auto col3 = cml::col(M, 3);
  CATCH_CHECK(col3.size() == 4);
  for(int i = 0; i < col3.size(); ++ i)
    CATCH_CHECK(col3[i] == M(i,3));
}

CATCH_TEST_CASE("dynamic row1")
{
  auto M = cml::matrixd(
    4,4,
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
    );

  auto row1 = cml::row(M, 1);
  CATCH_CHECK(row1.size() == 4);
  for(int j = 0; j < row1.size(); ++ j)
    CATCH_CHECK(row1[j] == M(1,j));
}

CATCH_TEST_CASE("dynamic col1")
{
  auto M = cml::matrixd(
    4,4,
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
    );

  auto col3 = cml::col(M, 3);
  CATCH_CHECK(col3.size() == 4);
  for(int i = 0; i < col3.size(); ++ i)
    CATCH_CHECK(col3[i] == M(i,3));
}

CATCH_TEST_CASE("mixed row1")
{
  auto v = cml::vector3d(1., 2., 3.);
  auto C = cml::outer(v,v);
  CATCH_REQUIRE(C.rows() == 3);
  CATCH_REQUIRE(C.cols() == 3);

  auto row1 = cml::row(C,1);
  for(int j = 0; j < row1.size(); ++ j)
    CATCH_CHECK(row1[j] == C(1,j));
}

CATCH_TEST_CASE("mixed col1")
{
  auto v = cml::vector3d(1., 2., 3.);
  auto C = cml::outer(v,v);
  CATCH_REQUIRE(C.rows() == 3);
  CATCH_REQUIRE(C.cols() == 3);

  auto col1 = cml::col(C,1);
  for(int i = 0; i < col1.size(); ++ i)
    CATCH_CHECK(col1[i] == C(i,1));
}

CATCH_TEST_CASE("mixed row2")
{
  auto M1 = cml::matrix22d(
    1., 2.,
    3., 4.
    );
  auto M2 = cml::matrix22d(
    5., 6.,
    7., 8.
    );

  auto M = M1 - M2;
  auto row2 = cml::row(M,1);
  for(int j = 0; j < row2.size(); ++ j)
    CATCH_CHECK(row2[j] == M(1,j));
}

CATCH_TEST_CASE("mixed col2")
{
  auto M1 = cml::matrix22d(
    1., 2.,
    3., 4.
    );
  auto M2 = cml::matrix22d(
    5., 6.,
    7., 8.
    );

  auto M = M1 - M2;
  auto col2 = cml::col(M,1);
  for(int i = 0; i < col2.size(); ++ i)
    CATCH_CHECK(col2[i] == M(i,1));
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
