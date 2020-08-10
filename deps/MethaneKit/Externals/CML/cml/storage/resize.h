/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_storage_resize_h
#define	cml_storage_resize_h

#include <cml/common/size_tags.h>

namespace cml {

/** Compute a new storage type by changing the size of @c Storage to @c N. */
template<class Storage, int N>
struct resize_storage
{
  /* Determine the new storage type: */
  typedef typename Storage::template resize<N>::type	type;
};

/** Convenience alias for resize_storage. */
template<class Storage, int N> using resize_storage_t
  = typename resize_storage<Storage, N>::type;


/** Compute a new storage type by changing the shape of @c Storage to @c R x
 * @c C.
 */
template<class Storage, int R, int C>
struct reshape_storage
{
  /* Determine the new storage type: */
  typedef typename Storage::template reshape<R,C>::type	type;
};

/** Convenience alias for reshape_storage. */
template<class Storage, int R, int C> using reshape_storage_t
  = typename reshape_storage<Storage, R, C>::type;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
