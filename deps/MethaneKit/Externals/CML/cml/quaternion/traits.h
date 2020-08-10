/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_traits_h
#define	cml_quaternion_traits_h

#include <cml/scalar/traits.h>
#include <cml/quaternion/type_util.h>

namespace cml {

/** Specializable class wrapping traits for quaternion<> types. This class
 * is used to simplify static polymorphism by providing a base class the
 * types used by a particular derived class.
 *
 * @tparam Quaternion The quaternion<> type the traits correspond to.
 */
template<class Quaternion> struct quaternion_traits;

/** traits_of for quaternion types. */
template<class Quaternion>
struct traits_of<Quaternion, enable_if_quaternion_t<Quaternion>> {
  typedef quaternion_traits<Quaternion>				type;
};

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
