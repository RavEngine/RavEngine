/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_memory_tags_h
#define	cml_common_memory_tags_h

namespace cml {

/** Specify a type containing compile-time memory. */
struct compiled_memory_tag {};

/** Specify a type containing allocated memory. */
struct allocated_memory_tag {};

/** Specify a type containing externally referenced memory (pointer,
 * reference, etc.).
 */
struct external_memory_tag {};

/** Specify a type having arbitrary or unspecified memory. */
struct any_memory_tag {};

/** Detect valid memory tags. */
template<class Tag> struct is_memory_tag {
  static const bool value
    =  std::is_same<Tag, compiled_memory_tag>::value
    || std::is_same<Tag, allocated_memory_tag>::value
    || std::is_same<Tag, external_memory_tag>::value
    || std::is_same<Tag, any_memory_tag>::value
    ;
};

/** Templated helper to determine the memory tag of an expression that
 * defines the memory_tag type.
 */
template<class T> struct memory_tag_of {
  typedef typename T::memory_tag type;
  static_assert(is_memory_tag<type>::value, "invalid memory tag");
};

/** Convenience alias for memory_tag_of. */
template<class T> using memory_tag_of_t = typename memory_tag_of<T>::type;

/** Helper to detect compiled-memory types. */
template<class Storage> struct is_compiled_memory {
  static const bool value =
    std::is_same<memory_tag_of_t<Storage>, compiled_memory_tag>::value;
};

/** Helper to detect allocated-memory types. */
template<class Storage> struct is_allocated_memory {
  static const bool value =
    std::is_same<memory_tag_of_t<Storage>, allocated_memory_tag>::value;
};

/** Helper to detect external-memory types. */
template<class Storage> struct is_external_memory {
  static const bool value =
    std::is_same<memory_tag_of_t<Storage>, external_memory_tag>::value;
};

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
