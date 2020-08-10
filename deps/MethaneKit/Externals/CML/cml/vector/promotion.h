/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_promotion_h
#define	cml_vector_promotion_h

#include <cml/common/promotion.h>
#include <cml/storage/resize.h>
#include <cml/storage/promotion.h>
#include <cml/scalar/promotion.h>
#include <cml/vector/type_util.h>
#include <cml/vector/traits.h>
#include <cml/vector/vector.h>

namespace cml {

/** Determine an appropriate storage type to use when combining vector
 * expressions via a pairwise binary operator.
 *
 * @note This can be specialized to accomodate user-defined storage types.
 */
template<class Storage1, class Storage2>
struct vector_binary_storage_promote
{
  static_assert(
    is_vector_storage<Storage1>::value &&
    is_vector_storage<Storage2>::value,
    "expected vector storage types for binary promotion");

  /* Determine the common unbound storage type: */
  typedef storage_promote_t<Storage1, Storage2>		unbound_type;

  /* Determine the new vector size: */
  static const int array_size
    = Storage1::array_size > Storage2::array_size
    ?  Storage1::array_size : Storage2::array_size;

  /* Resize: */
  typedef resize_storage_t<unbound_type, array_size>	resized_type;

  /* Rebind to a vector storage type: */
  typedef rebind_vector_storage_t<resized_type>		type;
};

/** Convenience alias for vector_binary_storage_promote. */
template<class Storage1, class Storage2>
  using vector_binary_storage_promote_t =
    typename vector_binary_storage_promote<Storage1, Storage2>::type;


/** Helper to deduce a reasonable vector type from two vector subexpression
 * types.  This can be specialized for non-default behavior.
 */
template<class Sub1, class Sub2, class Enable = void> struct vector_promote;

template<class Sub1, class Sub2> struct vector_promote<
  Sub1, Sub2, typename std::enable_if<
    is_vector<Sub1>::value && is_vector<Sub2>::value>::type>
{
  typedef cml::unqualified_type_t<Sub1>			left_type;
  typedef cml::unqualified_type_t<Sub2>			right_type;
  typedef vector_traits<left_type>			left_traits;
  typedef vector_traits<right_type>			right_traits;

  /* Deduce the new vector element type: */
  typedef value_type_promote_t<left_traits, right_traits> value_type;

  /* Determine the new storage type: */
  typedef vector_binary_storage_promote_t<
    storage_type_of_t<left_traits>,
    storage_type_of_t<right_traits>>			storage_type;

  /* Use the proxy type for the temporary: */
  typedef proxy_type_of_t<storage_type>			proxy_type;

  /* Build the vector type: */
  typedef vector<value_type, proxy_type>		type;
};

/** Convenience alias for vector_promote<>. */
template<class Sub1, class Sub2>
  using vector_promote_t = typename vector_promote<Sub1,Sub2>::type;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
