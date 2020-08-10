/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_size_tags_h
#define	cml_common_size_tags_h

#include <type_traits>
#include <cml/common/traits.h>

namespace cml {

/** Indicates a fixed-sized operand.
 *
 * @note Fixed-size vector types define a constant, @c array_size,
 * containing the array length.
 *
 * @note Fixed-size matrix types define two constants, @c array_rows and @c
 * array_cols, containing the number of rows and columns.
 */
struct fixed_size_tag {};

/** Indicates a runtime-sized operand.
 *
 * @note Dynamic-size vector types define a constant, @c array_size, set to
 * -1.
 *
 * @note Dynamic-size matrices define two constants, @c array_rows and @c
 * array_cols, both set to -1.
 */
struct dynamic_size_tag {};

/** Indicates that the vector or matrix type has an unspecified or
 * arbitrary size tag.
 */
struct any_size_tag {};

/** Detect valid size tags.
 *
 * @note This can be specialized for user-defined size tags.
 */
template<class Tag> struct is_size_tag {
  static const bool value
    =  std::is_same<Tag, fixed_size_tag>::value
    || std::is_same<Tag, dynamic_size_tag>::value
    || std::is_same<Tag, any_size_tag>::value;
};

/** Templated helper to determine the size tag of an expression that
 * defines the size_tag type.
 */
template<class T> struct size_tag_of {
  typedef typename T::size_tag type;
  static_assert(is_size_tag<type>::value, "invalid size tag");
};

/** Convenience alias for size_tag_of. */
template<class T> using size_tag_of_t = typename size_tag_of<T>::type;

/** Retrieve the size_tag of @c T via traits. */
template<class T> struct size_tag_trait_of {
  typedef typename traits_of<T>::type::size_tag type;
  static_assert(is_size_tag<type>::value, "invalid size tag");
};

/** Convenience alias for size_tag_trait_of. */
template<class T>
  using size_tag_trait_of_t = typename size_tag_trait_of<T>::type;

/** Helper to detect fixed-size types. */
template<class T> struct is_fixed_size {
  static const bool value =
    std::is_same<size_tag_of_t<T>, fixed_size_tag>::value;
};

/** Wrapper for enable_if to detect types tagged with fixed_size_tag. */
template<class Sub, class T = void> struct enable_if_fixed_size
: std::enable_if<is_fixed_size<Sub>::value, T> {};

/** Convenience alias for enable_if_fixed_size. */
template<class Sub, class T = void> using enable_if_fixed_size_t
  = typename enable_if_fixed_size<Sub, T>::type;


/** Helper to detect dynamic-size types. */
template<class T> struct is_dynamic_size {
  static const bool value =
    std::is_same<size_tag_of_t<T>, dynamic_size_tag>::value;
};

/** Wrapper for enable_if to detect types tagged with dynamic_size_tag. */
template<class Sub, class T = void> struct enable_if_dynamic_size
: std::enable_if<is_dynamic_size<Sub>::value, T> {};

/** Convenience alias for enable_if_dynamic_size. */
template<class Sub, class T = void> using enable_if_dynamic_size_t
  = typename enable_if_dynamic_size<Sub, T>::type;


/** Helper to detect any-size types. */
template<class T> struct is_any_size {
  static const bool value =
    std::is_same<size_tag_of_t<T>, any_size_tag>::value;
};

/** Wrapper for enable_if to detect types tagged with any_size_tag. */
template<class Sub, class T = void> struct enable_if_any_size
: std::enable_if<is_any_size<Sub>::value, T> {};

/** Convenience alias for enable_if_any_size. */
template<class Sub, class T = void> using enable_if_any_size_t
  = typename enable_if_any_size<Sub, T>::type;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
