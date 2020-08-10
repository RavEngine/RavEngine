/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file catch_main.cpp
 */

#define CATCH_CONFIG_RUNNER
#if defined(WIN32)
#define DO_NOT_USE_WMAIN
#endif
#include "catch.hpp"

int
main(int argc, char** argv)
{
  int result = Catch::Session().run(argc, argv);
  return result < 0xff ? result : 0xff;
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
