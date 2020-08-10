/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_temporary_h
#define	cml_vector_temporary_h

#include <cml/common/temporary.h>
#include <cml/storage/resize.h>
#include <cml/storage/promotion.h>
#include <cml/vector/traits.h>
#include <cml/vector/vector.h>

namespace cml {

/** Deduce a temporary for a vector expression. */
template<class Vector>
struct temporary_of< Vector, cml::enable_if_vector_t<Vector> >
{
  typedef cml::unqualified_type_t<Vector>		vector_type;

  /* Propagate the element type of the original vector: */
  typedef vector_traits<vector_type>			traits_type;
  typedef typename traits_type::value_type		value_type;
  typedef typename traits_type::storage_type		storage_type;

  /* Need the proxy for the storage type: */
  typedef proxy_type_of_t<storage_type>			proxy_type;

  /* Build the temporary: */
  typedef vector<value_type, proxy_type>		type;
};


/** Deduce a subvector temporary type for a vector expression. */
template<class Vector, typename std::enable_if<
  vector_traits<Vector>::array_size == vector_traits<Vector>::array_size
  >::type* = nullptr>
struct subvector_of
{
  typedef cml::unqualified_type_t<Vector>		vector_type;

  /* Propagate the element type of the original vector: */
  typedef vector_traits<vector_type>			traits_type;
  typedef typename traits_type::storage_type		storage_type;
  typedef typename traits_type::value_type		value_type;
  typedef typename traits_type::size_tag		size_tag;

  /* Compute the new storage size: */
  private:
  static const int old_array_size = traits_type::array_size;
  static const int new_array_size = old_array_size - 1;
  static const int N = new_array_size > 0 ? new_array_size : -1;
  public:

  /* Determine the new storage type: */
  typedef resize_storage_t<storage_type, N>		resized_type;
  typedef rebind_t<resized_type, vector_storage_tag>	rebound_type;
  typedef proxy_type_of_t<rebound_type>			proxy_type;

  /* Build the subvector from the new storage type: */
  typedef vector<value_type, proxy_type>		type;
};

/** Convenience alias for subvector_of. */
template<class T> using subvector_of_t = typename subvector_of<T>::type;


/** Deduce a supervector temporary type for a vector expression. */
template<class Vector, typename std::enable_if<
  vector_traits<Vector>::array_size == vector_traits<Vector>::array_size
  >::type* = nullptr>
struct supervector_of
{
  typedef cml::unqualified_type_t<Vector>		vector_type;

  /* Propagate the element type of the original vector: */
  typedef vector_traits<vector_type>			traits_type;
  typedef typename traits_type::storage_type		storage_type;
  typedef typename traits_type::value_type		value_type;
  typedef typename traits_type::size_tag		size_tag;

  /* Compute the new storage size: */
  private:
  static const int old_array_size = traits_type::array_size;
  static const int new_array_size = old_array_size + 1;
  static const int N = new_array_size > 0 ? new_array_size : -1;
  public:

  /* Determine the new storage type: */
  typedef resize_storage_t<storage_type, N>		resized_type;
  typedef rebind_t<resized_type, vector_storage_tag>	rebound_type;
  typedef proxy_type_of_t<rebound_type>			proxy_type;

  /* Build the supervector from the new storage type: */
  typedef vector<value_type, proxy_type>		type;
};

/** Convenience alias for supervector_of. */
template<class T> using supervector_of_t = typename supervector_of<T>::type;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
