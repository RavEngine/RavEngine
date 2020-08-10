/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_storage_tags_h
#define	cml_common_storage_tags_h

#include <type_traits>

namespace cml {

/** Specify vector storage types. */
struct vector_storage_tag {};

/** Specify matrix storage types. */
struct matrix_storage_tag {};

/** Specify quaternion storage types. */
struct quaternion_storage_tag {};

/** Detect valid storage tags. */
template<class Tag> struct is_storage_tag {
  static const bool value
    =  std::is_same<Tag, vector_storage_tag>::value
    || std::is_same<Tag, matrix_storage_tag>::value
    || std::is_same<Tag, quaternion_storage_tag>::value
    ;
};

/** Templated helper to determine the storage tag of an expression that
 * defines the storage_tag type.
 */
template<class T> struct storage_tag_of {
  typedef typename T::storage_tag type;
  static_assert(is_storage_tag<type>::value, "invalid storage tag");
};

/** Convenience alias for storage_tag_of. */
template<class T> using storage_tag_of_t = typename storage_tag_of<T>::type;

/** Helper to detect vector storage types. */
template<class Storage> struct is_vector_storage {
  static const bool value =
    std::is_same<storage_tag_of_t<Storage>, vector_storage_tag>::value;
};

/** Helper to detect matrix storage types. */
template<class Storage> struct is_matrix_storage {
  static const bool value =
    std::is_same<storage_tag_of_t<Storage>, matrix_storage_tag>::value;
};

/** Helper to detect quaternion storage types. */
template<class Storage> struct is_quaternion_storage {
  static const bool value =
    std::is_same<storage_tag_of_t<Storage>, quaternion_storage_tag>::value;
};

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
