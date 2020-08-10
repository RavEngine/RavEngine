/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_traits_h
#define	cml_vector_traits_h

#include <cml/scalar/traits.h>
#include <cml/vector/type_util.h>

namespace cml {

/** Specializable class wrapping traits for vector<> types. This class is
 * used to simplify static polymorphism by providing a base class the types
 * used by a particular derived class.
 *
 * @tparam Vector The vector<> type the traits correspond to.
 */
template<class Vector> struct vector_traits;

/** traits_of for vector types. */
template<class Vector>
struct traits_of<Vector, enable_if_vector_t<Vector>> {
  typedef vector_traits<Vector>				type;
};

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
