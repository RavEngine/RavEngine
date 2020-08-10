/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <iostream>
#include <type_traits>
#include <cml/common/compiler.h>

/* Testing headers: */
#include "catch_runner.h"

struct rv_from_this {
#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
  rv_from_this&& mover() &&;
#endif
  rv_from_this& refer() __CML_REF;
};

CATCH_TEST_CASE("rv_from_this1")
{
#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
  CATCH_CHECK(true ==
    (std::is_same<decltype(rv_from_this().mover()), rv_from_this&&>::value)
  );
  CATCH_REQUIRE(true ==
    (std::is_same<decltype(std::declval<rv_from_this&>().refer()), rv_from_this&>::value)
  );
#else
  CATCH_REQUIRE(true ==
    (std::is_same<decltype(std::declval<rv_from_this&>().refer()), rv_from_this&>::value)
  );
#endif
}

// -------------------------------------------------------------------------
// vim:ft=cpp:s=2
