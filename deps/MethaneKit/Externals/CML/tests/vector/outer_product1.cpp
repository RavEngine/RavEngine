/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/vector/outer_product.h>

#include <cml/vector.h>
#include <cml/matrix.h>
#include <cml/util/vector_print.h>
#include <cml/util/matrix_print.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("fixed, outer1")
{
  auto v = cml::vector3d(1., 2., 3.);
  auto C = cml::outer(v,v);
  CATCH_REQUIRE(C.rows() == 3);
  CATCH_REQUIRE(C.cols() == 3);

  auto expected = cml::matrix33d(
    1., 2., 3.,
    2., 4., 6.,
    3., 6., 9.
    );

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(C(i,j) == Approx(expected(i,j)).epsilon(1e-12));
}




CATCH_TEST_CASE("fixed external, outer1")
{
  double av[] = { 1., 2., 3. };
  auto v = cml::external3d(av);
  auto C = cml::outer(v,v);
  CATCH_REQUIRE(C.rows() == 3);
  CATCH_REQUIRE(C.cols() == 3);

  auto expected = cml::matrix33d(
    1., 2., 3.,
    2., 4., 6.,
    3., 6., 9.
    );

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(C(i,j) == Approx(expected(i,j)).epsilon(1e-12));
}




CATCH_TEST_CASE("dynamic external, outer1")
{
  double av[] = { 1., 2., 3. };
  auto v = cml::externalnd(3, av);
  auto C = cml::outer(v,v);
  CATCH_REQUIRE(C.rows() == 3);
  CATCH_REQUIRE(C.cols() == 3);

  auto expected = cml::matrix33d(
    1., 2., 3.,
    2., 4., 6.,
    3., 6., 9.
    );

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(C(i,j) == Approx(expected(i,j)).epsilon(1e-12));
}




CATCH_TEST_CASE("dynamic, outer1")
{
  auto v = cml::vectord(1., 2., 3.);
  auto C = cml::outer(v,v);
  CATCH_REQUIRE(C.rows() == 3);
  CATCH_REQUIRE(C.cols() == 3);

  auto expected = cml::matrix33d(
    1., 2., 3.,
    2., 4., 6.,
    3., 6., 9.
    );

  for(int i = 0; i < 3; ++ i)
    for(int j = 0; j < 3; ++ j)
      CATCH_CHECK(C(i,j) == Approx(expected(i,j)).epsilon(1e-12));
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
