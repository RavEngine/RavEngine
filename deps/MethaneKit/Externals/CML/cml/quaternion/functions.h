/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_functions_h
#define	cml_quaternion_functions_h

#include <cml/scalar/promotion.h>
#include <cml/quaternion/temporary.h>

namespace cml {

/** Return the real part of quaternion @c q. */
template<class DerivedT> auto
real(const readable_quaternion<DerivedT>& q)
-> value_type_trait_of_t<DerivedT>;

/** Return the squared length of @c q. */
template<class DerivedT> auto
length_squared(const readable_quaternion<DerivedT>& q)
-> value_type_trait_of_t<DerivedT>;

/** Return the length of @c q. */
template<class DerivedT> auto
length(const readable_quaternion<DerivedT>& q)
-> value_type_trait_of_t<DerivedT>;

/** Return the Cayley norm (squared length) of @c q. */
template<class DerivedT> auto
norm(const readable_quaternion<DerivedT>& q)
-> value_type_trait_of_t<DerivedT>;

/** Return a normalized copy of @c q. */
template<class DerivedT> auto
normalize(const readable_quaternion<DerivedT>& q)
-> temporary_of_t<DerivedT>;

/** Return the multiplicative identity of quaternion @c q. */
template<class DerivedT> auto
identity(const readable_quaternion<DerivedT>& q)
-> temporary_of_t<DerivedT>;

/** Return the log of @c q. */
template<class DerivedT> auto
log(const readable_quaternion<DerivedT>& q)
-> temporary_of_t<DerivedT>;

/** Return the exponential of @c q. */
template<class DerivedT> auto
exp(const readable_quaternion<DerivedT>& q)
-> temporary_of_t<DerivedT>;

} // namespace cml

#define __CML_QUATERNION_FUNCTIONS_TPP
#include <cml/quaternion/functions.tpp>
#undef __CML_QUATERNION_FUNCTIONS_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
