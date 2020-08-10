/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <typeinfo>
#include <iostream>

// Make sure the main header compiles cleanly:
#include <cml/matrix/promotion.h>

#include <cml/matrix/fixed.h>
#include <cml/matrix/external.h>
#include <cml/matrix/dynamic.h>
#include <cml/matrix/types.h>

/* Testing headers: */
#include "catch_runner.h"


CATCH_TEST_CASE("fixed fixed1")
{
  typedef cml::matrix_inner_product_promote<
    cml::matrix22d, cml::matrix22d>			promoter_type;
  typedef promoter_type::type				type;
  CATCH_CHECK((std::is_same<type, cml::matrix22d>::value));
}

CATCH_TEST_CASE("fixed dynamic1")
{
  typedef cml::matrix_inner_product_promote<
    cml::matrix22d, cml::matrixd>			promoter_type;
  typedef promoter_type::type				type;
  CATCH_CHECK((std::is_same<type, cml::matrixd>::value));
}

CATCH_TEST_CASE("fixed external1")
{
  typedef cml::matrix_inner_product_promote<
    cml::matrix22d, cml::external22d>			promoter_type;
  typedef promoter_type::type				type;
  CATCH_CHECK((std::is_same<type, cml::matrix22d>::value));
}

CATCH_TEST_CASE("fixed external2")
{
  typedef cml::matrix_inner_product_promote<
    cml::matrix22d, cml::externalmnd>			promoter_type;
  typedef promoter_type::type				type;
  CATCH_CHECK((std::is_same<type, cml::matrixd>::value));
}

CATCH_TEST_CASE("dynamic fixed1")
{
  typedef cml::matrix_inner_product_promote<
    cml::matrixd, cml::matrix22d>			promoter_type;
  typedef promoter_type::type				type;
  CATCH_CHECK((std::is_same<type, cml::matrixd>::value));
}

CATCH_TEST_CASE("dynamic dynamic1")
{
  typedef cml::matrix_inner_product_promote<
    cml::matrixd, cml::matrixd>				promoter_type;
  typedef promoter_type::type				type;
  CATCH_CHECK((std::is_same<type, cml::matrixd>::value));
}

CATCH_TEST_CASE("dynamic external1")
{
  typedef cml::matrix_inner_product_promote<
    cml::matrixd, cml::external22d>			promoter_type;
  typedef promoter_type::type				type;
  CATCH_CHECK((std::is_same<type, cml::matrixd>::value));
}

CATCH_TEST_CASE("dynamic external2")
{
  typedef cml::matrix_inner_product_promote<
    cml::matrixd, cml::externalmnd>			promoter_type;
  typedef promoter_type::type				type;
  CATCH_CHECK((std::is_same<type, cml::matrixd>::value));
}

CATCH_TEST_CASE("fixed external fixed1")
{
  typedef cml::matrix_inner_product_promote<
    cml::external22d, cml::matrix22d>			promoter_type;
  typedef promoter_type::type				type;
  CATCH_CHECK((std::is_same<type, cml::matrix22d>::value));
}

CATCH_TEST_CASE("fixed external dynamic1")
{
  typedef cml::matrix_inner_product_promote<
    cml::external22d, cml::matrixd>			promoter_type;
  typedef promoter_type::type				type;
  CATCH_CHECK((std::is_same<type, cml::matrixd>::value));
}

CATCH_TEST_CASE("fixed external external1")
{
  typedef cml::matrix_inner_product_promote<
    cml::external22d, cml::external22d>			promoter_type;
  typedef promoter_type::type				type;
  CATCH_CHECK((std::is_same<type, cml::matrix22d>::value));
}

CATCH_TEST_CASE("fixed external external2")
{
  typedef cml::matrix_inner_product_promote<
    cml::external22d, cml::externalmnd>			promoter_type;
  typedef promoter_type::type				type;
  CATCH_CHECK((std::is_same<type, cml::matrixd>::value));
}

CATCH_TEST_CASE("dynamic external fixed1")
{
  typedef cml::matrix_inner_product_promote<
    cml::externalmnd, cml::matrix22d>			promoter_type;
  typedef promoter_type::type				type;
  CATCH_CHECK((std::is_same<type, cml::matrixd>::value));
}

CATCH_TEST_CASE("dynamic external dynamic1")
{
  typedef cml::matrix_inner_product_promote<
    cml::externalmnd, cml::matrixd>			promoter_type;
  typedef promoter_type::type				type;
  CATCH_CHECK((std::is_same<type, cml::matrixd>::value));
}

CATCH_TEST_CASE("dynamic external external1")
{
  typedef cml::matrix_inner_product_promote<
    cml::externalmnd, cml::external22d>			promoter_type;
  typedef promoter_type::type				type;
  CATCH_CHECK((std::is_same<type, cml::matrixd>::value));
}

CATCH_TEST_CASE("dynamic external external2")
{
  typedef cml::matrix_inner_product_promote<
    cml::externalmnd, cml::externalmnd>			promoter_type;
  typedef promoter_type::type				type;
  CATCH_CHECK((std::is_same<type, cml::matrixd>::value));
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
