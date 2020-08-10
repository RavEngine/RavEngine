/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_mpl_is_statically_polymorphic_h
#define	cml_common_mpl_is_statically_polymorphic_h

#include <cml/common/mpl/if_t.h>

namespace cml {

/** Defines typedef @c type as std::true_type if @c T has a member function
 * actual() that returns a reference type, or std::false_type otherwise.
 * The static bool @c value is set to true or false to match @c type.
 */
template<class T> struct is_statically_polymorphic {
  private:

  /* SFINAE overload to deduce the return type of T::actual, if it exists. */
  template<class U> static auto get_type_of_actual(int)
    -> decltype(std::declval<U>().actual());

  /* The default overload deduces a void return type. */
  template<class U> static auto get_type_of_actual(...)
    -> void;


  public:

  /* std::true_type if T::actual is a member function returning a
   * reference type, std::false_type otherwise.
   */
  typedef cml::if_t<
    std::is_reference<decltype(get_type_of_actual<T>(0))>::value,
    std::true_type, std::false_type> type;

  /* True or false, depending upon 'type': */
  static const bool value = type::value;
};

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
