/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_temporary_h
#define	cml_quaternion_temporary_h

#include <cml/common/temporary.h>
#include <cml/storage/resize.h>
#include <cml/storage/promotion.h>
#include <cml/quaternion/traits.h>
#include <cml/quaternion/quaternion.h>

namespace cml {

/** Deduce a temporary for a quaternion expression. */
template<class Quaternion>
struct temporary_of< Quaternion, cml::enable_if_quaternion_t<Quaternion> >
{
  typedef cml::unqualified_type_t<Quaternion>		quaternion_type;

  /* Propagate the element type of the original quaternion: */
  typedef quaternion_traits<quaternion_type>		traits_type;
  typedef typename traits_type::value_type		value_type;
  typedef typename traits_type::storage_type		storage_type;
  typedef typename traits_type::order_type		order_type;
  typedef typename traits_type::cross_type		cross_type;

  /* Need the proxy for the storage type: */
  typedef proxy_type_of_t<storage_type>			proxy_type;

  /* Build the temporary: */
  typedef quaternion<value_type,
	  proxy_type, order_type, cross_type>		type;
};

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
