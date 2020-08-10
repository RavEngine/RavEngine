/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_traits_h
#define	cml_matrix_traits_h

#include <cml/common/traits.h>
#include <cml/matrix/type_util.h>

namespace cml {

/** Specializable class wrapping traits for matrix<> types. This class
 * is used to simplify static polymorphism by providing a polymorphic base
 * class the types used by a particular derived class.
 *
 * @tparam Matrix The matrix<> type the traits correspond to.
 */
template<class Matrix> struct matrix_traits;

/** traits_of for matrix types. */
template<class Matrix>
struct traits_of<Matrix, enable_if_matrix_t<Matrix>> {
  typedef matrix_traits<Matrix>				type;
};

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
