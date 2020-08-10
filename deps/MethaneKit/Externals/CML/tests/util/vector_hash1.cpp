/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <iostream>
#include <unordered_set>

// Make sure the main header compiles cleanly:
#include <cml/util/vector_hash.h>

#include <cml/vector/fixed.h>
#include <cml/vector/dynamic.h>
#include <cml/vector/external.h>
#include <cml/vector/types.h>
#include <cml/vector/comparison.h>

/* Testing headers: */
#include "catch_runner.h"


typedef std::unordered_set<cml::vector3d>		vector3d_table;

CATCH_TEST_CASE("fixed, table1")
{
  cml::vector3d p0(1.,2.,3.);
  vector3d_table table;
  CATCH_REQUIRE(table.insert(p0).second);
  CATCH_REQUIRE(table.size() == 1);
  CATCH_REQUIRE(table.find(p0) != table.end());
}



typedef std::unordered_set<cml::external3d>		external3d_table;

CATCH_TEST_CASE("fixed external, table1")
{
  double av0[] = { 1., 2., 3. };
  cml::external3d p0(av0);
  external3d_table table;
  CATCH_REQUIRE(table.insert(p0).second);
  CATCH_REQUIRE(table.size() == 1);
  CATCH_REQUIRE(table.find(p0) != table.end());
}



typedef std::unordered_set<cml::vectord>		vectord_table;

CATCH_TEST_CASE("dynamic, table1")
{
  cml::vectord p0(1.,2.,3.);
  vectord_table table;
  CATCH_REQUIRE(table.insert(p0).second);
  CATCH_REQUIRE(table.size() == 1);
  CATCH_REQUIRE(table.find(p0) != table.end());
}



typedef std::unordered_set<cml::externalnd>		externalnd_table;

CATCH_TEST_CASE("dynamic external, table1")
{
  double av0[] = { 1., 2., 3. };
  cml::externalnd p0(av0, 3);
  externalnd_table table;
  CATCH_REQUIRE(table.insert(p0).second);
  CATCH_REQUIRE(table.size() == 1);
  CATCH_REQUIRE(table.find(p0) != table.end());
}


// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
