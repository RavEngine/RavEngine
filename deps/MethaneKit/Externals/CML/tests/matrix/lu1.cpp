/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/matrix/lu.h>

#include <cml/vector.h>
#include <cml/matrix.h>

#include <cml/util/matrix_print.h>
#include <cml/util/vector_print.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("fixed, lu1")
{
  auto M = cml::matrix44d(
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
    );
  auto LU = cml::lu(M);
  double D = LU(0,0)*LU(1,1)*LU(2,2)*LU(3,3);
  CATCH_CHECK(D == Approx(-120.).epsilon(1e-12));

  auto expected = cml::matrix44d(
    2., 0., 2., .6,
    1.5, 3., 1., -2.9,
    2.5, (1.+2./3.), -(2.+2./3.), (5.+1./3.),
    -.5, (-2./3.), -1.9, 7.5
    );

  for(int i = 0; i < 4; ++ i)
    for(int j = 0; j < 4; ++ j)
      CATCH_CHECK(LU(i,j) == Approx(expected(i,j)).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, lu_pivot1")
{
  auto M = cml::matrix44d(
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
    );
  auto lup = cml::lu_pivot(M);
  CATCH_CHECK(lup.sign == -1);

  auto expected = cml::matrix44d(
     5.,  5., 4.,  2.,
     .4, -2., .4, -.2,
    -.2,  .5, 4., -.5,
     .6,  0., .4, -3.
    );
  for(int i = 0; i < 4; ++ i)
    for(int j = 0; j < 4; ++ j)
      CATCH_CHECK(lup.lu(i,j) == Approx(expected(i,j)).epsilon(1e-12));

  std::array<int,4> order = { 2, 0, 3, 1 };
  for(int i = 0; i < 4; ++ i)
    CATCH_CHECK(lup.order[i] == order[i]);
}

CATCH_TEST_CASE("fixed, lu_pivot2")
{
  auto M = cml::matrix44d(
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
    );

  cml::lu_pivot_result<cml::matrix44d> lup(M);
  cml::lu_pivot(lup);
  CATCH_CHECK(lup.sign == -1);

  auto expected = cml::matrix44d(
     5.,  5., 4.,  2.,
     .4, -2., .4, -.2,
    -.2,  .5, 4., -.5,
     .6,  0., .4, -3.
    );
  for(int i = 0; i < 4; ++ i)
    for(int j = 0; j < 4; ++ j)
      CATCH_CHECK(lup.lu(i,j) == Approx(expected(i,j)).epsilon(1e-12));

  std::array<int,4> order = { 2, 0, 3, 1 };
  for(int i = 0; i < 4; ++ i)
    CATCH_CHECK(lup.order[i] == order[i]);
}


CATCH_TEST_CASE("fixed, lu_solve1")
{
  auto A = cml::matrix44d(
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
    );
  auto LU = cml::lu(A);
  auto b = cml::vector4d(5., 1., 8., 3.);
  auto x = cml::lu_solve(LU, b);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, lu_solve2")
{
  auto A = cml::matrix44d(
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
    );
  auto LU = cml::lu(A);
  auto b = cml::vector4d(5., 1., 8., 3.);
  auto x = cml::vector4d();
  cml::lu_solve(LU, x, b);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}


CATCH_TEST_CASE("fixed, lu_pivot_solve1")
{
  auto A = cml::matrix44d(
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
    );
  auto lup = cml::lu_pivot(A);
  CATCH_CHECK(lup.sign == -1);

  auto b = cml::vector4d(5., 1., 8., 3.);
  auto x = cml::lu_solve(lup, b);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, lu_pivot_solve2")
{
  auto A = cml::matrix44d(
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
    );
  auto lup = cml::lu_pivot(A);
  CATCH_CHECK(lup.sign == -1);

  auto b = cml::vector4d(5., 1., 8., 3.);
  auto x = cml::vector4d();
  cml::lu_solve(lup, x, b);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed, lu_pivot_solve3")
{
  auto A = cml::matrix44d(
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
    );
  auto lup = cml::lu_pivot(A);
  CATCH_CHECK(lup.sign == -1);

  auto b = cml::vector4d(5., 1., 8., 3.);
  auto x = b;
  cml::lu_solve(lup, x, x);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}




CATCH_TEST_CASE("fixed external, lu1")
{
  double avM[] = {
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
  };
  auto M = cml::external44d(avM);
  auto LU = cml::lu(M);
  double D = LU(0,0)*LU(1,1)*LU(2,2)*LU(3,3);
  CATCH_CHECK(D == Approx(-120.).epsilon(1e-12));

  auto expected = cml::matrix44d(
    2., 0., 2., .6,
    1.5, 3., 1., -2.9,
    2.5, (1.+2./3.), -(2.+2./3.), (5.+1./3.),
    -.5, (-2./3.), -1.9, 7.5
    );

  for(int i = 0; i < 4; ++ i)
    for(int j = 0; j < 4; ++ j)
      CATCH_CHECK(LU(i,j) == Approx(expected(i,j)).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed external, lu_pivot1")
{
  double avM[] = {
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
  };
  auto M = cml::external44d(avM);
  auto lup = cml::lu_pivot(M);
  CATCH_CHECK(lup.sign == -1);

  auto expected = cml::matrix44d(
     5.,  5., 4.,  2.,
     .4, -2., .4, -.2,
    -.2,  .5, 4., -.5,
     .6,  0., .4, -3.
    );
  for(int i = 0; i < 4; ++ i)
    for(int j = 0; j < 4; ++ j)
      CATCH_CHECK(lup.lu(i,j) == Approx(expected(i,j)).epsilon(1e-12));

  std::array<int,4> order = { 2, 0, 3, 1 };
  for(int i = 0; i < 4; ++ i)
    CATCH_CHECK(lup.order[i] == order[i]);
}

CATCH_TEST_CASE("fixed external, lu_pivot2")
{
  double avM[] = {
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
  };
  auto M = cml::external44d(avM);

  cml::lu_pivot_result<cml::matrix44d> lup(M);
  cml::lu_pivot(lup);
  CATCH_CHECK(lup.sign == -1);

  auto expected = cml::matrix44d(
     5.,  5., 4.,  2.,
     .4, -2., .4, -.2,
    -.2,  .5, 4., -.5,
     .6,  0., .4, -3.
    );
  for(int i = 0; i < 4; ++ i)
    for(int j = 0; j < 4; ++ j)
      CATCH_CHECK(lup.lu(i,j) == Approx(expected(i,j)).epsilon(1e-12));

  std::array<int,4> order = { 2, 0, 3, 1 };
  for(int i = 0; i < 4; ++ i)
    CATCH_CHECK(lup.order[i] == order[i]);
}


CATCH_TEST_CASE("fixed external, lu_solve1")
{
  double avM[] = {
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
  };
  auto A = cml::external44d(avM);
  auto LU = cml::lu(A);

  double avb[] = { 5., 1., 8., 3. };
  auto b = cml::external4d(avb);
  auto x = cml::lu_solve(LU, b);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed external, lu_solve2")
{
  double avM[] = {
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
  };
  auto A = cml::external44d(avM);
  auto LU = cml::lu(A);

  double avb[] = { 5., 1., 8., 3. };
  auto b = cml::external4d(avb);

  double avx[4];
  auto x = cml::external4d(avx);

  cml::lu_solve(LU, x, b);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}


CATCH_TEST_CASE("fixed external, lu_pivot_solve1")
{
  double avM[] = {
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
  };
  auto A = cml::external44d(avM);
  auto lup = cml::lu_pivot(A);
  CATCH_CHECK(lup.sign == -1);

  double avb[] = { 5., 1., 8., 3. };
  auto b = cml::external4d(avb);
  auto x = cml::lu_solve(lup, b);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed external, lu_pivot_solve2")
{
  double avM[] = {
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
  };
  auto A = cml::external44d(avM);
  auto lup = cml::lu_pivot(A);
  CATCH_CHECK(lup.sign == -1);

  double avb[] = { 5., 1., 8., 3. };
  auto b = cml::external4d(avb);

  double avx[4];
  auto x = cml::external4d(avx);

  cml::lu_solve(lup, x, b);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}

CATCH_TEST_CASE("fixed external, lu_pivot_solve3")
{
  double avM[] = {
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
  };
  auto A = cml::external44d(avM);
  auto lup = cml::lu_pivot(A);
  CATCH_CHECK(lup.sign == -1);

  double avb[] = { 5., 1., 8., 3. };
  auto b = cml::external4d(avb);

  double avx[4] = { 5., 1., 8., 3. };
  auto x = cml::external4d(avx);

  cml::lu_solve(lup, x, x);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}




CATCH_TEST_CASE("dynamic external, lu1")
{
  double avM[] = {
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
  };
  auto M = cml::externalmnd(4,4, avM);
  auto LU = cml::lu(M);
  double D = LU(0,0)*LU(1,1)*LU(2,2)*LU(3,3);
  CATCH_CHECK(D == Approx(-120.).epsilon(1e-12));

  auto expected = cml::matrix44d(
    2., 0., 2., .6,
    1.5, 3., 1., -2.9,
    2.5, (1.+2./3.), -(2.+2./3.), (5.+1./3.),
    -.5, (-2./3.), -1.9, 7.5
    );

  for(int i = 0; i < 4; ++ i)
    for(int j = 0; j < 4; ++ j)
      CATCH_CHECK(LU(i,j) == Approx(expected(i,j)).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic external, lu_pivot1")
{
  double avM[] = {
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
  };
  auto M = cml::externalmnd(4,4, avM);
  auto lup = cml::lu_pivot(M);
  CATCH_CHECK(lup.sign == -1);

  auto expected = cml::matrix44d(
     5.,  5., 4.,  2.,
     .4, -2., .4, -.2,
    -.2,  .5, 4., -.5,
     .6,  0., .4, -3.
    );
  for(int i = 0; i < 4; ++ i)
    for(int j = 0; j < 4; ++ j)
      CATCH_CHECK(lup.lu(i,j) == Approx(expected(i,j)).epsilon(1e-12));

  std::array<int,4> order = { 2, 0, 3, 1 };
  for(int i = 0; i < 4; ++ i)
    CATCH_CHECK(lup.order[i] == order[i]);
}

CATCH_TEST_CASE("dynamic external, lu_pivot2")
{
  double avM[] = {
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
  };
  auto M = cml::externalmnd(4,4, avM);

  cml::lu_pivot_result<cml::matrix44d> lup(M);
  cml::lu_pivot(lup);
  CATCH_CHECK(lup.sign == -1);

  auto expected = cml::matrix44d(
     5.,  5., 4.,  2.,
     .4, -2., .4, -.2,
    -.2,  .5, 4., -.5,
     .6,  0., .4, -3.
    );
  for(int i = 0; i < 4; ++ i)
    for(int j = 0; j < 4; ++ j)
      CATCH_CHECK(lup.lu(i,j) == Approx(expected(i,j)).epsilon(1e-12));

  std::array<int,4> order = { 2, 0, 3, 1 };
  for(int i = 0; i < 4; ++ i)
    CATCH_CHECK(lup.order[i] == order[i]);
}


CATCH_TEST_CASE("dynamic external, lu_solve1")
{
  double avM[] = {
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
  };
  auto A = cml::externalmnd(4,4, avM);
  auto LU = cml::lu(A);

  double avb[] = { 5., 1., 8., 3. };
  auto b = cml::externalnd(4, avb);

  auto x = cml::lu_solve(LU, b);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic external, lu_solve2")
{
  double avM[] = {
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
  };
  auto A = cml::external44d(avM);
  auto LU = cml::lu(A);

  double avb[] = { 5., 1., 8., 3. };
  auto b = cml::externalnd(4, avb);

  double avx[4];
  auto x = cml::externalnd(4, avx);

  cml::lu_solve(LU, x, b);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}


CATCH_TEST_CASE("dynamic external, lu_pivot_solve1")
{
  double avM[] = {
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
  };
  auto A = cml::external44d(avM);
  auto lup = cml::lu_pivot(A);
  CATCH_CHECK(lup.sign == -1);

  double avb[] = { 5., 1., 8., 3. };
  auto b = cml::externalnd(4, avb);

  auto x = cml::lu_solve(lup, b);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic external, lu_pivot_solve2")
{
  double avM[] = {
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
  };
  auto A = cml::external44d(avM);
  auto lup = cml::lu_pivot(A);
  CATCH_CHECK(lup.sign == -1);

  double avb[] = { 5., 1., 8., 3. };
  auto b = cml::externalnd(4, avb);

  double avx[4];
  auto x = cml::externalnd(4, avx);

  cml::lu_solve(lup, x, b);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic external, lu_pivot_solve3")
{
  double avM[] = {
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
  };
  auto A = cml::external44d(avM);
  auto lup = cml::lu_pivot(A);
  CATCH_CHECK(lup.sign == -1);

  double avb[] = { 5., 1., 8., 3. };
  auto b = cml::externalnd(4, avb);

  double avx[] = { 5., 1., 8., 3. };
  auto x = cml::externalnd(4, avx);

  cml::lu_solve(lup, x, x);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}




CATCH_TEST_CASE("dynamic, lu1")
{
  auto M = cml::matrixd(
    4,4,
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
    );
  auto LU = cml::lu(M);
  double D = LU(0,0)*LU(1,1)*LU(2,2)*LU(3,3);
  CATCH_CHECK(D == Approx(-120.).epsilon(1e-12));

  auto expected = cml::matrix44d(
    2., 0., 2., .6,
    1.5, 3., 1., -2.9,
    2.5, (1.+2./3.), -(2.+2./3.), (5.+1./3.),
    -.5, (-2./3.), -1.9, 7.5
    );

  for(int i = 0; i < 4; ++ i)
    for(int j = 0; j < 4; ++ j)
      CATCH_CHECK(LU(i,j) == Approx(expected(i,j)).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic, lu_pivot1")
{
  auto M = cml::matrixd(
    4,4,
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
    );
  auto lup = cml::lu_pivot(M);
  CATCH_CHECK(lup.sign == -1);

  auto expected = cml::matrix44d(
     5.,  5., 4.,  2.,
     .4, -2., .4, -.2,
    -.2,  .5, 4., -.5,
     .6,  0., .4, -3.
    );
  for(int i = 0; i < 4; ++ i)
    for(int j = 0; j < 4; ++ j)
      CATCH_CHECK(lup.lu(i,j) == Approx(expected(i,j)).epsilon(1e-12));

  std::array<int,4> order = { 2, 0, 3, 1 };
  for(int i = 0; i < 4; ++ i)
    CATCH_CHECK(lup.order[i] == order[i]);
}

CATCH_TEST_CASE("dynamic, lu_pivot2")
{
  auto M = cml::matrixd(
    4,4,
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
    );

  cml::lu_pivot_result<cml::matrixd> lup(M);
  cml::lu_pivot(lup);
  CATCH_CHECK(lup.sign == -1);

  auto expected = cml::matrix44d(
     5.,  5., 4.,  2.,
     .4, -2., .4, -.2,
    -.2,  .5, 4., -.5,
     .6,  0., .4, -3.
    );
  for(int i = 0; i < 4; ++ i)
    for(int j = 0; j < 4; ++ j)
      CATCH_CHECK(lup.lu(i,j) == Approx(expected(i,j)).epsilon(1e-12));

  std::array<int,4> order = { 2, 0, 3, 1 };
  for(int i = 0; i < 4; ++ i)
    CATCH_CHECK(lup.order[i] == order[i]);
}


CATCH_TEST_CASE("dynamic, lu_solve1")
{
  auto A = cml::matrixd(
    4,4,
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
    );
  auto LU = cml::lu(A);
  auto b = cml::vectord(5., 1., 8., 3.);
  auto x = cml::lu_solve(LU, b);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic, lu_solve2")
{
  auto A = cml::matrixd(
    4,4,
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
    );
  auto LU = cml::lu(A);
  auto b = cml::vectord(5., 1., 8., 3.);
  auto x = cml::vectord(4);
  cml::lu_solve(LU, x, b);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}


CATCH_TEST_CASE("dynamic, lu_pivot_solve1")
{
  auto A = cml::matrixd(
    4,4,
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
    );
  auto lup = cml::lu_pivot(A);
  CATCH_CHECK(lup.sign == -1);

  auto b = cml::vectord(5., 1., 8., 3.);
  auto x = cml::lu_solve(lup, b);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic, lu_pivot_solve2")
{
  auto A = cml::matrixd(
    4,4,
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
    );
  auto lup = cml::lu_pivot(A);
  CATCH_CHECK(lup.sign == -1);

  auto b = cml::vectord(5., 1., 8., 3.);
  auto x = cml::vectord(4);
  cml::lu_solve(lup, x, b);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}

CATCH_TEST_CASE("dynamic, lu_pivot_solve3")
{
  auto A = cml::matrixd(
    4,4,
     2.,  0.,  2.,  .6,
     3.,  3.,  4.,  -2.,
     5.,  5.,  4.,   2.,
    -1., -2., 3.4,  -1.
    );
  auto lup = cml::lu_pivot(A);
  CATCH_CHECK(lup.sign == -1);

  auto b = cml::vectord(5., 1., 8., 3.);
  auto x = b;
  cml::lu_solve(lup, x, x);
  auto Ax = A*x;
  for(int i = 0; i < 4; ++ i) CATCH_CHECK(Ax[i] == Approx(b[i]).epsilon(1e-12));
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
