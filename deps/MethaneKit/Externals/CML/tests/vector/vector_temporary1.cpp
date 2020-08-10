/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <cml/vector.h>

/* Testing headers: */
#include "catch_runner.h"

/* Errors: */

/* temporary_of<> relied upon vector_traits<>, which is not specialized for
 * const types:
 */
CATCH_TEST_CASE("const_temporary1")
{
  typedef cml::temporary_of_t<const cml::vector3d> const_type;
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
