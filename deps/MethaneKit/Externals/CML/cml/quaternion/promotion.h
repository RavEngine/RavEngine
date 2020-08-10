/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_promotion_h
#define	cml_quaternion_promotion_h

#include <cml/common/promotion.h>
#include <cml/storage/resize.h>
#include <cml/storage/promotion.h>
#include <cml/scalar/promotion.h>
#include <cml/quaternion/order_tags.h>
#include <cml/quaternion/cross_tags.h>
#include <cml/quaternion/type_util.h>
#include <cml/quaternion/traits.h>
#include <cml/quaternion/quaternion.h>

namespace cml {

/** Determine an appropriate storage type to use when combining quaternion
 * expressions via a pairwise binary operator.
 *
 * @note This can be specialized to accomodate user-defined storage types.
 */
template<class Storage1, class Storage2>
struct quaternion_binary_storage_promote
{
  static_assert(
    is_quaternion_storage<Storage1>::value &&
    is_quaternion_storage<Storage2>::value,
    "expected quaternion storage types for binary promotion");

  /* Determine the common unbound storage type: */
  typedef storage_promote_t<Storage1, Storage2>		unbound_type;

  /* Resize: */
  typedef resize_storage_t<unbound_type,4>		resized_type;

  /* Rebind to a quaternion storage type: */
  typedef rebind_quaternion_storage_t<resized_type>	type;
};

/** Convenience alias for quaternion_binary_storage_promote. */
template<class Storage1, class Storage2>
  using quaternion_binary_storage_promote_t =
    typename quaternion_binary_storage_promote<Storage1, Storage2>::type;


/** Helper to deduce a reasonable quaternion type from two quaternion
 * subexpression types.  This can be specialized for non-default behavior.
 */
template<class Sub1, class Sub2, class Enable = void>
  struct quaternion_promote;

template<class Sub1, class Sub2> struct quaternion_promote<
  Sub1, Sub2, typename std::enable_if<
    is_quaternion<Sub1>::value && is_quaternion<Sub2>::value>::type>
{
  typedef cml::unqualified_type_t<Sub1>			left_type;
  typedef cml::unqualified_type_t<Sub2>			right_type;
  typedef quaternion_traits<left_type>			left_traits;
  typedef quaternion_traits<right_type>			right_traits;

  /* Deduce the new quaternion element type: */
  typedef value_type_promote_t<left_traits, right_traits> value_type;

  /* Determine the new storage type: */
  typedef quaternion_binary_storage_promote_t<
    storage_type_of_t<left_traits>,
    storage_type_of_t<right_traits>>			storage_type;

  /* Use the proxy type for the temporary: */
  typedef proxy_type_of_t<storage_type>			proxy_type;

  /* Determine the common order type: */
  typedef order_type_promote_t<
    order_type_of_t<left_traits>,
    order_type_of_t<right_traits>>			order_type;

  /* Determine the common cross type: */
  typedef cross_type_promote_t<
    cross_type_of_t<left_traits>,
    cross_type_of_t<right_traits>>			cross_type;

  /* Build the quaternion type: */
  typedef quaternion<value_type,
	  proxy_type, order_type, cross_type>		type;
};

/** Convenience alias for quaternion_promote<>. */
template<class Sub1, class Sub2>
  using quaternion_promote_t = typename quaternion_promote<Sub1,Sub2>::type;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
