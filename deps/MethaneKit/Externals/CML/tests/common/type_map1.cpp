/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <cml/common/mpl/type_map.h>

/* Testing headers: */
#include "catch_runner.h"

template<class T1, class T2>
struct table_item
{
  typedef T1 first;
  typedef T2 second;
};

CATCH_TEST_CASE("map1")
{
  using cml::type_map;
  typedef type_map<
    /**/  table_item<int,int>
    ,     table_item<float,int>
    ,     table_item<double,int>
    > int_table;

  typedef int_table::find_second<int>::type Ti;
  CATCH_CHECK(Ti::value);
  CATCH_CHECK((std::is_same<Ti::type, int>::value));

  typedef int_table::find_first<int>::type Ti2;
  CATCH_CHECK(Ti2::value);
  CATCH_CHECK((std::is_same<Ti2::type, int>::value));

  typedef int_table::find_first<float>::type Tf1;
  CATCH_CHECK(Tf1::value);
  CATCH_CHECK((std::is_same<Tf1::type, int>::value));

  typedef int_table::find_second<float>::type Tf2;
  CATCH_CHECK(!Tf2::value);
  CATCH_CHECK((std::is_same<Tf2::type, void>::value));

  typedef int_table::find_first<double>::type Td1;
  CATCH_CHECK(Td1::value);
  CATCH_CHECK((std::is_same<Td1::type, int>::value));

  typedef int_table::find_second<double>::type Td2;
  CATCH_CHECK(!Td2::value);
  CATCH_CHECK((std::is_same<Td2::type, void>::value));
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
