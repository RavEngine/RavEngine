/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_util_h
#define	cml_util_h

#include <cml/scalar/functions.h>

namespace cml {

/** Convert horizontal field of view to vertical field of view. */
template<typename T> inline T xfov_to_yfov(T xfov, T aspect) {
  return T(2*scalar_traits<T>::atan(scalar_traits<T>::tan(xfov/2) / aspect));
}

/** Convert vertical field of view to horizontal field of view. */
template<typename T> inline T yfov_to_xfov(T yfov, T aspect) {
  return T(2*scalar_traits<T>::atan(scalar_traits<T>::tan(yfov/2) * aspect));
}

/** Convert horizontal zoom to vertical zoom. */
template<typename T> inline T xzoom_to_yzoom(T xzoom, T aspect) {
  return xzoom * aspect;
}

/** Convert vertical zoom to horizontal zoom. */
template<typename T> inline T yzoom_to_xzoom(T yzoom, T aspect) {
  return yzoom / aspect;
}

/** Convert zoom factor to field of view. */
template<typename T> inline T zoom_to_fov(T zoom) {
  return T(2*scalar_traits<T>::atan(T(1) / zoom));
}

/** Convert field of view to zoom factor. */
template<typename T> inline T fov_to_zoom(T fov) {
  return T(1) / scalar_traits<T>::tan(fov/2);
}

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
