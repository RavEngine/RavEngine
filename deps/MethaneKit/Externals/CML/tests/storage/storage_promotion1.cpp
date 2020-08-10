/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#include <iostream>
#include <type_traits>
#include <cml/storage/promotion.h>

/* Testing headers: */
#include "catch_runner.h"

template<class S1, class S2>
  using promote_t = typename cml::storage_promote<S1, S2>::type;

template<class S1, class S2, class S>
  using check_c = std::is_same<promote_t<S1, S2>, S>;

CATCH_TEST_CASE("basic1")
{
  using cml::any_storage;
  using cml::compiled;
  using cml::external;
  using cml::allocated;
  using cml::vector_storage_tag;
  using cml::rebind_t;

  typedef rebind_t<compiled<>, vector_storage_tag>	compiled_type;
  typedef rebind_t<allocated<>, vector_storage_tag>	allocated_type;
  typedef rebind_t<external<>, vector_storage_tag>	external_type;
  typedef rebind_t<any_storage<>, vector_storage_tag>	any_type;

#define _CHECK(_S1, _S2, _S)						\
  CATCH_CHECK((check_c<_S1, _S2, _S>::value))

    _CHECK( compiled_type,	compiled_type,	compiled<>	 );
    _CHECK( compiled_type,	allocated_type,	compiled<>	 );
    _CHECK( compiled_type,	external_type,	compiled<>	 );
    _CHECK( compiled_type,	any_type,	compiled<>	 );
    _CHECK( allocated_type,	allocated_type,	allocated<>	 );
    _CHECK( allocated_type,	external_type,	allocated<>	 );
    _CHECK( allocated_type,	any_type,	allocated<>	 );
    _CHECK( external_type,	external_type,	external<>	 );
    _CHECK( external_type,	any_type,	external<>	 );
    _CHECK( any_type,		any_type,	any_storage<>	 );
#undef _CHECK
}

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
