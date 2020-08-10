/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_mpl_type_table_h
#define	cml_common_mpl_type_table_h

#include <cml/common/mpl/if_t.h>

#ifdef _MSC_VER
/* Disable "identifier too long" warning: */
#pragma warning(push)
#pragma warning(disable : 4503)
#endif

namespace cml {

/** A symmetric mapping of type pairs to a new type, using a list of
 * arbitrary items as the source of the mapping.  Types in @c Entries must
 * define @c first, @c second, and @c type, where @c first and @c second
 * are the map-from types, and @c type is the map-to type.  For example:
 *
 * @code
 * template<class T1, class T2, class T> struct item {
 * 	typedef T1 first;
 * 	typedef T2 second;
 * 	typedef T type;
 * };
 * @endcode
 */
template<class... Entries>
class type_table
{
  public:

    /** The type of @c type_table::find<T1,T2>::type. */
    template<class T, bool Found> struct result_type {

      /** The found type.  Only valid if @c value is true. */
      typedef T type;

      /** @c value is true if @c type is in the table, false otherwise. */
      enum { value = Found };
    };


  public:

    /** Type-compare <@c Ta,@c Tb> to <@c Entry::first,@c Entry::second>
     * using std::is_same, ignoring order. @c value is set to the
     * comparison result.
     */
    template<class Ta, class Tb, class Entry>
      struct match
      {
	typedef typename Entry::first			first;
	typedef typename Entry::second			second;
	static const bool value
	  =  (std::is_same<Ta,first>::value && std::is_same<Tb,second>::value)
	  || (std::is_same<Ta,second>::value && std::is_same<Tb,first>::value);
      };


  private:

    /** The internal find<> helper. */
    template<typename T1, typename T2, class... Rest> struct _find;


  public:

    /** Search the table for type pair <@c T1, @c T2>.  If the pair is
     * found, @c type is set to <@c T, true>, where @c T is the mapped-to
     * type.  Otherwise, @c type is set to <void,false>.
     */
    template<typename T1, typename T2> struct find {
      typedef typename _find<T1, T2, Entries...>::type	type;
    };


  private:

    /** Match <@c T1, @c T2> to @c Entry, and recursively to @c Rest. */
    template<typename T1, typename T2, class Entry, class... Rest>
      struct _find<T1,T2,Entry,Rest...>
      {
	static const bool found = match<T1,T2, Entry>::value;
	typedef cml::if_t<found
	  , result_type<typename Entry::type, true>
	  , typename _find<T1, T2, Rest...>::type>	type;
      };

    /** The not-found case. */
    template<typename T1, typename T2>
      struct _find<T1,T2>
      {
	typedef result_type<void, false>		type;
      };
};

/** Basic implementation of a type_table entry. */
template<class T1, class T2, class T>
struct type_table_item
{
  typedef T1 first;
  typedef T2 second;
  typedef T type;
};

} // namespace cml

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
