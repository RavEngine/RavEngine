/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_quaternion_h
#define	cml_quaternion_quaternion_h

#include <cml/quaternion/cross_tags.h>
#include <cml/quaternion/order_tags.h>

namespace cml {

/** Specializable class for building quaternions.
 *
 * This class encapsulates the notion of a quaternion.
 *
 * @note Quaternions with two different orders cannot be used in the same
 * expression.
 *
 * @tparam Element The scalar type for quaternion elements, with the
 * following operators defined: +, -, *, /, <, >, ==, = (assign).
 *
 * @tparam StorageType Storage type to use for holding the array of quaternion
 * elements.
 *
 * @tparam Order Specifies the position of the scalar and imaginary
 * elements when accessed like a vector.
 *
 * @tparam Cross Specifies whether the cross product of two quaternions is
 * left-handed (negative_cross) or right-handed (positive_cross).
 */
template<typename Element, class ArrayType = fixed<>,
    class Order = imaginary_first, class Cross = positive_cross>
      class quaternion;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
