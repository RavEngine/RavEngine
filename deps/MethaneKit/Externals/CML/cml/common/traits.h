/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_traits_h
#define	cml_common_traits_h

namespace cml {

/** Specializable struct to obtain the traits of a specified class,
 * possibly using SFINAE.  Specializations should typedef "type" as the
 * traits class for @c T.
 */
template<class T, typename Enable = void> struct traits_of;

/** Helper alias for traits_of. */
template<class T> using traits_of_t = typename traits_of<T>::type;

/** @addtogroup container_traits */
/*@{*/

/** Retrieve the value_type of @c T via an embedded typedef. */
template<class T> struct value_type_of {
  typedef typename T::value_type			type;
};

/** Convenience alias for value_typet_of. */
template<class T> using value_type_of_t = typename value_type_of<T>::type;

/** Retrieve the value_type of  @c T via traits.
 *
 * @note This applies to CML expression types, including scalars.
 */
template<class T> struct value_type_trait_of {
  typedef typename traits_of<T>::type::value_type	type;
};

/** Convenience alias for value_type_trait_of. */
template<class T>
  using value_type_trait_of_t = typename value_type_trait_of<T>::type;

/*@}*/

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
