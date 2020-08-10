/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_storage_any_selector_h
#define	cml_storage_any_selector_h

#include <cml/common/size_tags.h>
#include <cml/common/storage_tags.h>
#include <cml/common/memory_tags.h>
#include <cml/storage/type_util.h>

namespace cml {

/** Base selector to specify an arbitrary or unknown storage type.
 *
 * @tparam Tag Tag specifying the type of storage (e.g.
 * vector_storage_tag).  This is set by instantiating @c rebind with the
 * required tag.
 */
template<class Tag = void> struct any_storage {

  /** Rebind the base selector to the required type. */
  template<class Rebind> struct rebind {
    typedef any_storage<Rebind>				other;
  };

  /** Make a partially bound selector by resizing. The new size is ignored
   * for any_storage<>.
   */
  template<int> struct resize {
    typedef any_storage<>				type;
  };

  /** Make a partially bound selector by reshaping.  The new shape is ignored
   * for any_storage<>.
   */
  template<int, int> struct reshape {
    typedef any_storage<>				type;
  };
};

/** Specialized selector for any vector storage. */
template<> struct any_storage<vector_storage_tag> {
  typedef any_storage<>					selector_type;
  typedef any_storage<>					unbound_type;
  typedef any_storage<>					proxy_type;
  typedef vector_storage_tag				storage_tag;
  typedef any_size_tag					size_tag;
  typedef any_memory_tag				memory_tag;

  /** Unspecified array size. */
  static const int array_size = -1;

  /** Rebind to a new vector any_storage<> selector (the template parameter
   * is ignored).
   */
  template<int> struct resize {
    typedef any_storage<>				type;
  };
};

/** Specialized selector for any matrix storage. */
template<> struct any_storage<matrix_storage_tag> {
  typedef any_storage<>					selector_type;
  typedef any_storage<>					unbound_type;
  typedef any_storage<>					proxy_type;
  typedef matrix_storage_tag				storage_tag;
  typedef any_size_tag					size_tag;
  typedef any_memory_tag				memory_tag;

  /** Unspecified number of array rows. */
  static const int array_rows = -1;

  /** Unspecified number of array columns. */
  static const int array_cols = -1;

  /** Rebind to a new matrix any_storage<> selector (the template
   * parameters are ignored).
   */
  template<int, int> struct reshape {
    typedef any_storage<>				type;
  };
};

/** Specialized selector for any quaternion storage. */
template<> struct any_storage<quaternion_storage_tag> {
  typedef any_storage<>					selector_type;
  typedef any_storage<>					unbound_type;
  typedef any_storage<>					proxy_type;
  typedef quaternion_storage_tag			storage_tag;
  typedef any_size_tag					size_tag;
  typedef any_memory_tag				memory_tag;

  /** Unspecified array size. */
  static const int array_size = -1;

  /** Rebind to a new vector any_storage<> selector (the template parameter
   * is ignored).
   */
  template<int> struct resize {
    typedef any_storage<>				type;
  };
};

/** is_storage_selector for any_storage<>. */
template<class Tag>
struct is_storage_selector<any_storage<Tag>> {
  static const bool value = true;
};

/** Helper to detect any_storage<> types. */
template<class Storage>
struct is_any_storage {
  static const bool value
    = std::is_same<typename Storage::selector_type, any_storage<>>::value;
};

/** Helper to disambiguate any_storage<> types. */
template<class Tag1, class Tag2>
struct storage_disambiguate<any_storage<Tag1>, any_storage<Tag2>>
{
  typedef any_storage<>					type;
};
 
} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
