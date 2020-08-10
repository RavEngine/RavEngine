/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/matrix/transpose.h>

#include <cml/matrix/fixed.h>
#include <cml/matrix/dynamic.h>
#include <cml/matrix/external.h>
#include <cml/matrix/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("fixed transpose_assign_3x3")
{
  auto M = cml::matrix33d(
    1.,  2.,  3.,
    1.,  4.,  9.,
    1., 16., 25.
    );

  M.transpose();
  auto expected = cml::matrix33d(
    1., 1.,  1.,
    2., 4., 16.,
    3., 9., 25.
    );

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(M(i,j) == expected(i,j));
}

CATCH_TEST_CASE("fixed transpose_3x3_1")
{
  auto M = cml::matrix33d(
    1.,  2.,  3.,
    1.,  4.,  9.,
    1., 16., 25.
    );

  auto T = cml::transpose(M);
  auto expected = cml::matrix33d(
    1., 1.,  1.,
    2., 4., 16.,
    3., 9., 25.
    );

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(T(i,j) == expected(i,j));
}

CATCH_TEST_CASE("fixed transpose_3x3_2")
{
  auto T = cml::transpose(
    cml::matrix33d(
      1.,  2.,  3.,
      1.,  4.,  9.,
      1., 16., 25.
      )
    );

  auto expected = cml::matrix33d(
    1., 1.,  1.,
    2., 4., 16.,
    3., 9., 25.
    );

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(T(i,j) == expected(i,j));
}

CATCH_TEST_CASE("fixed external transpose_assign_3x3")
{
  double avM[] = {
    1.,  2.,  3.,
    1.,  4.,  9.,
    1., 16., 25.
  };
  cml::external33d M(avM);

  M.transpose();
  auto expected = cml::matrix33d(
    1., 1.,  1.,
    2., 4., 16.,
    3., 9., 25.
    );

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(M(i,j) == expected(i,j));
}

CATCH_TEST_CASE("fixed external transpose_3x3_1")
{
  double avM[] = {
    1.,  2.,  3.,
    1.,  4.,  9.,
    1., 16., 25.
  };
  cml::external33d M(avM);

  auto T = cml::transpose(M);
  auto expected = cml::matrix33d(
    1., 1.,  1.,
    2., 4., 16.,
    3., 9., 25.
    );

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(T(i,j) == expected(i,j));
}

CATCH_TEST_CASE("fixed external transpose_3x3_2")
{
  double avM[] = {
    1.,  2.,  3.,
    1.,  4.,  9.,
    1., 16., 25.
  };

  auto T = cml::transpose(cml::external33d(avM));
  auto expected = cml::matrix33d(
    1., 1.,  1.,
    2., 4., 16.,
    3., 9., 25.
    );

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(T(i,j) == expected(i,j));
}

CATCH_TEST_CASE("dynamic external transpose_assign_3x3")
{
  double avM[] = {
    1.,  2.,  3.,
    1.,  4.,  9.,
    1., 16., 25.
  };
  cml::externalmnd M(3,3, avM);

  M.transpose();
  auto expected = cml::matrix33d(
    1., 1.,  1.,
    2., 4., 16.,
    3., 9., 25.
    );

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(M(i,j) == expected(i,j));
}

CATCH_TEST_CASE("dynamic external transpose_3x3_1")
{
  double avM[] = {
    1.,  2.,  3.,
    1.,  4.,  9.,
    1., 16., 25.
  };
  cml::externalmnd M(3,3, avM);

  auto T = cml::transpose(M);
  auto expected = cml::matrix33d(
    1., 1.,  1.,
    2., 4., 16.,
    3., 9., 25.
    );

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(T(i,j) == expected(i,j));
}

CATCH_TEST_CASE("dynamic external transpose_3x3_2")
{
  double avM[] = {
    1.,  2.,  3.,
    1.,  4.,  9.,
    1., 16., 25.
  };

  auto T = cml::transpose(cml::externalmnd(3,3, avM));
  auto expected = cml::matrix33d(
    1., 1.,  1.,
    2., 4., 16.,
    3., 9., 25.
    );

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(T(i,j) == expected(i,j));
}

CATCH_TEST_CASE("dynamic transpose_assign_3x3")
{
  auto M = cml::matrixd(
    3,3,
    1.,  2.,  3.,
    1.,  4.,  9.,
    1., 16., 25.
    );

  M.transpose();
  auto expected = cml::matrix33d(
    1., 1.,  1.,
    2., 4., 16.,
    3., 9., 25.
    );

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(M(i,j) == expected(i,j));
}

CATCH_TEST_CASE("dynamic transpose_3x3_1")
{
  auto M = cml::matrixd(
    3,3,
    1.,  2.,  3.,
    1.,  4.,  9.,
    1., 16., 25.
    );

  auto T = cml::transpose(M);
  auto expected = cml::matrix33d(
    1., 1.,  1.,
    2., 4., 16.,
    3., 9., 25.
    );

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(T(i,j) == expected(i,j));
}

CATCH_TEST_CASE("dynamic transpose_3x3_2")
{
  auto T = cml::transpose(
    cml::matrixd(
      3,3,
      1.,  2.,  3.,
      1.,  4.,  9.,
      1., 16., 25.
      )
    );

  auto expected = cml::matrix33d(
    1., 1.,  1.,
    2., 4., 16.,
    3., 9., 25.
    );

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(T(i,j) == expected(i,j));
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
