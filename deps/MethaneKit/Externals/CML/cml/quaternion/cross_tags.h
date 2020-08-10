/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_cross_tags_h
#define	cml_quaternion_cross_tags_h

namespace cml {

/** Helper to specify v1^v2 multiplication cross. */
struct positive_cross {};

/** Helper to specify v2^v1 multiplication cross. */
struct negative_cross {};

/** Detect valid cross types. */
template<class Tag> struct is_cross_type {
  static const bool value
    =  std::is_same<Tag, positive_cross>::value
    || std::is_same<Tag, negative_cross>::value
    ;
};

/** Templated helper to determine the cross type of an expression that
 * defines the cross_type type.
 */
template<class T> struct cross_type_of {
  typedef typename T::cross_type type;
  static_assert(is_cross_type<type>::value, "invalid cross type");
};

/** Convenience alias for cross_type_of. */
template<class T> using cross_type_of_t = typename cross_type_of<T>::type;

/** Retrieve the cross_type of @c T via traits. */
template<class T> struct cross_type_trait_of {
  typedef typename traits_of<T>::type::cross_type type;
  static_assert(is_cross_type<type>::value, "invalid cross type");
};

/** Convenience alias for cross_type_trait_of. */
template<class T>
  using cross_type_trait_of_t = typename cross_type_trait_of<T>::type;


/** Deduce the default cross tag needed to promote the result of combining
 * two expressions having crosss @c Tag1 and @c Tag2.  By default:
 *
 * - both imaginary_first: imaginary_first
 * - both real_first: real_first
 * - otherwise: compile-time error
 */
template<class Tag1, class Tag2> struct cross_type_promote
{
  static_assert(is_cross_type<Tag1>::value,
    "invalid quaternion cross type");
  static_assert(is_cross_type<Tag2>::value,
    "invalid quaternion cross type");
  static_assert(std::is_same<Tag1,Tag2>::value,
    "mismatched quaternion cross types");

  /* The tags are the same, so use the common type: */
  typedef Tag1						type;
};

/** Convenience alias for cross_type_promote. */
template<class Tag1, class Tag2>
  using cross_type_promote_t = typename cross_type_promote<Tag1,Tag2>::type;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
