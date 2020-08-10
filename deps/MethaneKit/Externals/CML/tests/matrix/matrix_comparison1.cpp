/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/matrix/comparison.h>

#include <cml/matrix/fixed.h>
#include <cml/matrix/types.h>

// For Catch:
#include <cml/util/matrix_print.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("equal1")
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

  CATCH_CHECK(M == expected);
}

CATCH_TEST_CASE("not_equal1")
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
    3., 9., 24.
    );

  CATCH_CHECK(M != expected);
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
