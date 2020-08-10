/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <cml/common/mpl/type_table.h>

/* Testing headers: */
#include "catch_runner.h"

template<class T1, class T2, class T>
struct table_item
{
  typedef T1 first;
  typedef T2 second;
  typedef T type;
};

CATCH_TEST_CASE("map1")
{
  using cml::type_table;
  typedef type_table<
    /**/  table_item<int,int,    int    >
    ,     table_item<int,float,  float  >
    ,     table_item<int,double, double >
    > int_table;

  typedef int_table::find<int,int>::type Tii;
  CATCH_CHECK(Tii::value);

  typedef int_table::find<int,double>::type Tid;
  CATCH_CHECK(Tid::value);

  typedef int_table::find<double,int>::type Tdi;
  CATCH_CHECK(Tdi::value);

  typedef int_table::find<int,char>::type Tic;
  CATCH_CHECK(!Tic::value);

  typedef int_table::find<char,int>::type Tic;
  CATCH_CHECK(!Tic::value);
}


struct map_struct {
  template<class T1, class T2, class T>
    struct table_item
    {
      typedef T1 first;
      typedef T2 second;
      typedef T type;
    };

  typedef cml::type_table<
    /**/  table_item<int,int,    int    >
    ,     table_item<int,float,  float  >
    ,     table_item<int,double, double >
    > int_table;
};

CATCH_TEST_CASE("map_struct1")
{
  typedef map_struct::int_table int_table;

  typedef int_table::find<int,int>::type Tii;
  CATCH_CHECK(Tii::value);

  typedef int_table::find<int,double>::type Tid;
  CATCH_CHECK(Tid::value);

  typedef int_table::find<double,int>::type Tdi;
  CATCH_CHECK(Tdi::value);

  typedef int_table::find<int,char>::type Tic;
  CATCH_CHECK(!Tic::value);

  typedef int_table::find<char,int>::type Tic;
  CATCH_CHECK(!Tic::value);
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
