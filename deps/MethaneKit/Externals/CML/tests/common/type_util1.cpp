/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <cml/common/type_util.h>

/* Testing headers: */
#include "catch_runner.h"

struct scoop1 { const scoop1& actual() const; };
struct scoop2 { scoop2& actual() const; };
struct scoop3 { scoop3& actual(); };
struct scoop4 { const scoop4& actual(); };

template<class T> struct scoop_base { T& actual(); };
struct scoop5 : scoop_base<scoop5> {};

CATCH_TEST_CASE("is_statically_polymorphic1")
{
  CATCH_CHECK(!cml::is_statically_polymorphic<int>::value);
}

CATCH_TEST_CASE("is_statically_polymorphic2")
{
  CATCH_CHECK(cml::is_statically_polymorphic<scoop1>::value);
  CATCH_CHECK(cml::is_statically_polymorphic<scoop2>::value);
  CATCH_CHECK(cml::is_statically_polymorphic<scoop3>::value);
  CATCH_CHECK(cml::is_statically_polymorphic<scoop4>::value);
}

CATCH_TEST_CASE("actual_type_of1")
{
  CATCH_CHECK((std::is_same<
      cml::actual_type_of_t<int>, int>::value));
  CATCH_CHECK((std::is_same<
      cml::actual_type_of_t<int&>, int>::value));
  CATCH_CHECK((std::is_same<
      cml::actual_type_of_t<const int&>, int>::value));
  CATCH_CHECK((std::is_same<
      cml::actual_type_of_t<int&&>, int>::value));
}

CATCH_TEST_CASE("actual_type_of2")
{
  CATCH_CHECK((std::is_same<
      cml::actual_type_of_t<scoop5>, scoop5>::value));
  CATCH_CHECK((std::is_same<
      cml::actual_type_of_t<scoop_base<scoop5>>, scoop5>::value));
}

CATCH_TEST_CASE("actual_operand_type_of1")
{
  CATCH_CHECK((std::is_same<cml::actual_operand_type_of_t<
      int&>, int&>::value));
  CATCH_CHECK((std::is_same<cml::actual_operand_type_of_t<
      int const&>, int const&>::value));
  CATCH_CHECK((std::is_same<cml::actual_operand_type_of_t<
      int&&>, int&&>::value));
}

CATCH_TEST_CASE("actual_operand_type_of2")
{
  CATCH_CHECK((std::is_same<cml::actual_operand_type_of_t<
      scoop_base<scoop5>&>, scoop5&>::value));
  CATCH_CHECK((std::is_same<cml::actual_operand_type_of_t<
      scoop_base<scoop5> const&>, scoop5 const&>::value));
  CATCH_CHECK((std::is_same<cml::actual_operand_type_of_t<
      scoop_base<scoop5>&&>, scoop5&&>::value));
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
