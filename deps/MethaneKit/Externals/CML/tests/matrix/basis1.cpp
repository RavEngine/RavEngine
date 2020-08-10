/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/matrix/basis.h>

#include <cml/matrix/row_col.h>
#include <cml/vector.h>
#include <cml/matrix.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("fixed basis1")
{
  // col basis:
  auto M = cml::matrix44d(
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
    );

  auto basis = cml::basis(M, 1);
  CATCH_CHECK(basis.size() == 4);
  for(int i = 0; i < basis.size(); ++ i)
    CATCH_CHECK(basis[i] == M.basis_element(1,i));
}

CATCH_TEST_CASE("fixed basis2")
{
  auto M = cml::matrix44d_r(
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
    );

  auto basis = cml::basis(M, 2);
  CATCH_CHECK(basis.size() == 4);
  for(int i = 0; i < basis.size(); ++ i)
    CATCH_CHECK(basis[i] == M.basis_element(2,i));
}

CATCH_TEST_CASE("fixed external basis1")
{
  double avM[16] = {
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
  };
  auto M = cml::external44d(avM);

  auto basis = cml::basis(M, 1);
  CATCH_CHECK(basis.size() == 4);
  for(int i = 0; i < basis.size(); ++ i)
    CATCH_CHECK(basis[i] == M.basis_element(1,i));
}

CATCH_TEST_CASE("fixed external basis2")
{
  double avM[16] = {
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
  };
  auto M = cml::external44d(avM);

  auto basis = cml::basis(M, 2);
  CATCH_CHECK(basis.size() == 4);
  for(int i = 0; i < basis.size(); ++ i)
    CATCH_CHECK(basis[i] == M.basis_element(2,i));
}

CATCH_TEST_CASE("dynamic external basis1")
{
  double avM[16] = {
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
  };
  auto M = cml::externalmnd(4,4, avM);

  auto basis = cml::basis(M, 1);
  CATCH_CHECK(basis.size() == 4);
  for(int i = 0; i < basis.size(); ++ i)
    CATCH_CHECK(basis[i] == M.basis_element(1,i));
}

CATCH_TEST_CASE("dynamic external basis2")
{
  double avM[16] = {
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
  };
  auto M = cml::externalmnd(4,4, avM);

  auto basis = cml::basis(M, 2);
  CATCH_CHECK(basis.size() == 4);
  for(int i = 0; i < basis.size(); ++ i)
    CATCH_CHECK(basis[i] == M.basis_element(2,i));
}

CATCH_TEST_CASE("dynamic basis1")
{
  auto M = cml::matrixd(
    4,4,
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
    );

  auto basis = cml::basis(M, 1);
  CATCH_CHECK(basis.size() == 4);
  for(int i = 0; i < basis.size(); ++ i)
    CATCH_CHECK(basis[i] == M.basis_element(1,i));
}

CATCH_TEST_CASE("dynamic basis2")
{
  auto M = cml::matrixd(
    4,4,
    1.,  2.,  3., 4.,
    1.,  4.,  9., 16.,
    1., 16., 25., 36.,
    1., 36., 81., 100.
    );

  auto basis = cml::basis(M, 2);
  CATCH_CHECK(basis.size() == 4);
  for(int i = 0; i < basis.size(); ++ i)
    CATCH_CHECK(basis[i] == M.basis_element(2,i));
}

CATCH_TEST_CASE("mixed basis3")
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
  auto basis = cml::basis(M,1);
  for(int i = 0; i < basis.size(); ++ i)
    CATCH_CHECK(basis[i] == M.basis_element(i,1));
}

CATCH_TEST_CASE("mixed basis2")
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
  auto basis = cml::basis(M,1);
  for(int i = 0; i < basis.size(); ++ i)
    CATCH_CHECK(basis[i] == M.basis_element(1,i));
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
