/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_order_tags_h
#define	cml_quaternion_order_tags_h

#include <cml/common/traits.h>

namespace cml {

/** Helper to specify real-first quaternion element ordering. */
struct real_first {
    enum { W = 0, X = 1, Y = 2, Z = 3 };
};

/** Helper to specify imaginary-first quaternion element ordering. */
struct imaginary_first {
    enum { X = 0, Y = 1, Z = 2, W = 3 };
};

/** Detect valid order types. */
template<class Tag> struct is_order_type {
  static const bool value
    =  std::is_same<Tag, real_first>::value
    || std::is_same<Tag, imaginary_first>::value
    ;
};

/** Templated helper to determine the order type of an expression that
 * defines the order_type type.
 */
template<class T> struct order_type_of {
  typedef typename T::order_type type;
  static_assert(is_order_type<type>::value, "invalid order type");
};

/** Convenience alias for order_type_of. */
template<class T> using order_type_of_t = typename order_type_of<T>::type;

/** Retrieve the order_type of @c T via traits. */
template<class T> struct order_type_trait_of {
  typedef typename traits_of<T>::type::order_type type;
  static_assert(is_order_type<type>::value, "invalid order type");
};

/** Convenience alias for order_type_trait_of. */
template<class T>
  using order_type_trait_of_t = typename order_type_trait_of<T>::type;


/** Deduce the default order tag needed to promote the result of combining
 * two expressions having orders @c Tag1 and @c Tag2.  By default:
 *
 * - both imaginary_first: imaginary_first
 * - both real_first: real_first
 * - otherwise: compile-time error
 */
template<class Tag1, class Tag2> struct order_type_promote
{
  static_assert(is_order_type<Tag1>::value,
    "invalid quaternion order type");
  static_assert(is_order_type<Tag2>::value,
    "invalid quaternion order type");
  static_assert(std::is_same<Tag1,Tag2>::value,
    "mismatched quaternion order types");

  /* The tags are the same, so use the common type: */
  typedef Tag1						type;
};

/** Convenience alias for order_type_promote. */
template<class Tag1, class Tag2>
  using order_type_promote_t = typename order_type_promote<Tag1,Tag2>::type;


/** For CML1 compatibility. */
using scalar_first = real_first;

/** For CML1 compatibility. */
using vector_first = imaginary_first;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
