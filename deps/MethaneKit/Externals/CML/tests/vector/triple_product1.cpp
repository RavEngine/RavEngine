/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

// Make sure the main header compiles cleanly:
#include <cml/vector/triple_product.h>

#include <cml/vector/fixed.h>
#include <cml/vector/dynamic.h>
#include <cml/vector/fixed_external.h>
#include <cml/vector/types.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("triple_product1")
{
  cml::vector3d v1 = { 1., 2., 3. };
  cml::vectord v2 = { 4., 5., 6. };
  double a3[3] = { 7., 8., 9. };
  cml::external3d v3(a3);
  double tp = cml::triple_product(v1,v2,v3);
  CATCH_CHECK(tp == 0.);
}

CATCH_TEST_CASE("size_check1")
{
  cml::vectord v1 = { 1., 2. };
  cml::vectord v2 = { 4., 5., 6. };
  double a3[3] = { 7., 8., 9. };
  cml::external3d v3(a3);
  CATCH_CHECK_THROWS_AS(cml::triple_product(v1,v2,v3), cml::vector_size_error);
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
