/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <cml/common/temporary.h>
#include <cml/scalar/traits.h>

/* Testing headers: */
#include "catch_runner.h"

CATCH_TEST_CASE("const_temporary1")
{
  typedef cml::temporary_of_t<const double> const_type;
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
