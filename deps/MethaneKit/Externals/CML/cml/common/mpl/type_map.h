/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_mpl_type_map_h
#define	cml_common_mpl_type_map_h

#include <cml/common/mpl/if_t.h>

#ifdef _MSC_VER
/* Disable "identifier too long" warning: */
#pragma warning(push)
#pragma warning(disable : 4503)
#endif

namespace cml {

/** A bijective mapping of unique types, using a list of arbitrary items as
 * the source of the mapping.  Types in @c Entries must define @c first and
 * @c second, where @c first and @c second are the mapped types.  For
 * example:
 *
 * @code
 * template<class T1, class T2> struct item {
 * 	typedef T1 first;
 * 	typedef T2 second;
 * };
 * @endcode
 */
template<class... Entries>
class type_map
{
  public:

    /** The type of @c type_map::find<T>::type. */
    template<class T, bool Found> struct result_type {

      /** The found type. Only valid if @c value is true. */
      typedef T type;

      /** @c value is true if @c type is in the table, false otherwise. */
      enum { value = Found };
    };


  public:

    /** Type-compare @c T to @c Entry::first.  @c value is set to the
     * comparison result.
     */
    template<class T, class Entry> struct match {
      typedef typename Entry::first first;
      static const bool value = std::is_same<T,first>::value;
    };


  private:

    /** The internal find<> helper. */
    template<template<class> class Map,
      typename T, class... Rest> struct _find;

    template<class E> struct map_first {
      typedef typename E::first first;
      typedef typename E::second second;
    };

    template<class E> struct map_second {
      typedef typename E::second first;
      typedef typename E::first second;
    };


  public:

    /** Search the @c first type of the map entries for T.  If the match is
     * found, @c type is set to <@c T2, true>, where @c T2 is the @c first
     * type of the matching Entry.  Otherwise, @c type is set to
     * <void,false>.
     */
    template<typename T> struct find_first {
      typedef typename _find<map_first, T, Entries...>::type type;
    };

    template<typename T> struct find_second {
      typedef typename _find<map_second, T, Entries...>::type type;
    };


  private:

    /** Match @c T to @c Entry, and recursively to @c Rest. */
    template<template<class> class Map,
      typename T, class Entry, class... Rest>
      struct _find<Map, T, Entry, Rest...>
      {
	typedef Map<Entry> map_type;
	static const bool found = match<T, map_type>::value;
	typedef cml::if_t<found
	  , result_type<typename map_type::second, true>
	  , typename _find<Map, T, Rest...>::type>	type;
      };

    /** The not-found case. */
    template<template<class> class Map, typename T> struct _find<Map, T> {
      typedef result_type<void, false>			type;
    };
};

/** Basic implementation of a type_map entry. */
template<class T1, class T2>
struct type_map_item
{
  typedef T1 first;
  typedef T2 second;
};

} // namespace cml

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
