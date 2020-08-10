/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_coordinate_conversion_h
#define	cml_mathlib_coordinate_conversion_h

#include <cml/scalar/traits.h>
#include <cml/vector/fwd.h>

/** @defgroup mathlib_coord_conversion Coordinate Conversion */

namespace cml {

/** @addtogroup mathlib_coord_conversion */
/*@{*/

/** Spherical conversion types. */
enum LatitudeType { latitude, colatitude };

/** For CML1 compatibility. */
typedef LatitudeType SphericalType;

/** @addtogroup mathlib_coord_conversion_to_cartesion Conversions to Cartesian Coordinates
 */
/*@{*/

/** Convert 2D polar coordinates to Cartesian coordinates.
 *
 * @throws vector_size_error at run-time if @c v is dynamically-sized, and
 * is not 2D.  If @c m is fixed-size, the size is checked at compile-time.
 */
template<class Sub, class E0, class E1> void
polar_to_cartesian(writable_vector<Sub>& v, E0 radius, E1 theta);

/** For CML1 compatibility. */
template<class E, class Sub> void
polar_to_cartesian(E radius, E theta, writable_vector<Sub>& v);


/** Convert 3D cylindrical coordinates to Cartesian coordinates. @c v[axis]
 * is set @c height, while the other two are set to the Cartesian
 * coordinates given by @c radius and @c theta.
 *
 * @param radius the cylinder radius.
 *
 * @param theta the angle around the cylinder axis to the point.
 *
 * @param height the distance from the cylinder base plane to the point.
 *
 * @throws vector_size_error at run-time if @c v is dynamically-sized, and
 * is not 3D.  If @c m is fixed-size, the size is checked at compile-time.
 *
 * @throws std::invalid_argument if @c axis is not 0, 1, or 2.
 */
template<class Sub, class E0, class E1, class E2> void
cylindrical_to_cartesian(
    writable_vector<Sub>& v, int axis, E0 radius, E1 theta, E2 height);

/** For CML1 compatibility. */
template<class E, class Sub> void
cylindrical_to_cartesian(
    E radius, E theta, E height, int axis, writable_vector<Sub>& v);


/** Convert 3D spherical coordinates to Cartesian coordinates.
 *
 * @param radius the distance from the origin to the point.
 *
 * @param theta the inclination.
 *
 * @param phi the azimuth.
 *
 * @throws vector_size_error at run-time if @c v is dynamically-sized, and
 * is not 3D.  If @c m is fixed-size, the size is checked at compile-time.
 *
 * @throws std::invalid_argument if @c axis is not 0, 1, or 2.
 */
template<class Sub, class E0, class E1, class E2> void
spherical_to_cartesian(writable_vector<Sub>& v,
  int axis, LatitudeType type, E0 radius, E1 theta, E2 phi);

/** For CML1 compatibility. */
template<class E, class Sub> void
spherical_to_cartesian(E radius, E theta, E phi,
  int axis, LatitudeType type, writable_vector<Sub>& v);

/*@}*/


/** @addtogroup mathlib_coord_conversion_from_cartesion Conversions from Cartesian Coordinates
 */
/*@{*/

/** Convert 2D Cartesian coordinates to polar coordinates.
 *
 * @throws vector_size_error at run-time if @c v is dynamically-sized, and
 * is not 2D.  If @c m is fixed-size, the size is checked at compile-time.
 *
 * @note @c theta will be 0 if @c radius < sqrt(eps), where eps is machine
 * epsilon for the value_type of @c Sub.
 */
template<class Sub, class E0, class E1> void
cartesian_to_polar(const readable_vector<Sub>& v, E0& radius, E1& theta);

/** Convert 2D Cartesian coordinates to polar coordinates, specifying a
 * zero tolerance on @c radius.
 */
template<class Sub, class E0, class E1, class Tol> void
cartesian_to_polar(const readable_vector<Sub>& v, E0& radius, E1& theta,
  Tol tolerance);


/** Convert 3D Cartesian coordinates to cylindrical coordinates.  @c height
 * is set to @c v[axis], while @c radius and @c theta are computed from the
 * other coordinates of @c v.
 *
 * @param radius the distance from the cylinder axis to @c v, measured on
 * the cylinder base plane.
 *
 * @param theta the angle around the cylinder axis to @c v.
 *
 * @param height the distance from the cylinder base plane to @c v.
 *
 * @throws vector_size_error at run-time if @c v is dynamically-sized, and
 * is not 3D.  If @c m is fixed-size, the size is checked at compile-time.
 *
 * @throws std::invalid_argument if @c axis is not 0, 1, or 2.
 *
 * @note @c theta will be 0 if @c radius < sqrt(eps), where eps is machine
 * epsilon for the value_type of @c Sub.
 */
template<class Sub, class E0, class E1, class E2> void
cartesian_to_cylindrical(const readable_vector<Sub>& v,
  int axis, E0& radius, E1& theta, E2& height);

/** Convert 3D Cartesian coordinates to cylindrical coordinates, specifying
 * a zero tolerance on @c radius.
 */
template<class Sub, class E0, class E1, class E2, class Tol> void
cartesian_to_cylindrical(const readable_vector<Sub>& v,
  int axis, E0& radius, E1& theta, E2& height, Tol tolerance);

/** For compatibility with CML1. */
template<class Sub, class E> void
cartesian_to_cylindrical(const readable_vector<Sub>& v,
  E& radius, E& theta, E& height, int axis,
  E tolerance = CML_DUMMY_TYPENAME scalar_traits<E>::sqrt_epsilon());


/** Convert 3D Cartesian coordinates to spherical coordinates.
 *
 * @param radius the distance from the origin to @c v.
 *
 * @param theta the inclination (angle from the axis base plane).
 *
 * @param phi the azimuth (angle around the axis).
 *
 * @throws vector_size_error at run-time if @c v is dynamically-sized, and
 * is not 3D.  If @c m is fixed-size, the size is checked at compile-time.
 *
 * @throws std::invalid_argument if @c axis is not 0, 1, or 2.
 *
 * @note @c theta will be 0 @c v lies on @c axis.
 *
 * @note @c phi will be 0 if @c v lies on the base plane.
 */
template<class Sub, class E0, class E1, class E2> void
cartesian_to_spherical(const readable_vector<Sub>& v,
  int axis, LatitudeType type, E0& radius, E1& theta, E2& phi);

/** Convert 3D Cartesian coordinates to spherical coordinates, specifying
 * the zero tolerance on @c theta.
 */
template<class Sub, class E0, class E1, class E2, class Tol> void
cartesian_to_spherical(const readable_vector<Sub>& v,
  int axis, LatitudeType type, E0& radius, E1& theta, E2& phi,
  Tol tolerance);

/** For compatibility with CML1. */
template<class Sub, class E> void
cartesian_to_spherical(const readable_vector<Sub>& v,
  E& radius, E& theta, E& phi, int axis, LatitudeType type,
  E tolerance = CML_DUMMY_TYPENAME scalar_traits<E>::sqrt_epsilon());

/*@}*/

/*@}*/

} // namespace cml

#define __CML_MATHLIB_COORDINATE_CONVERSION_TPP
#include <cml/mathlib/coordinate_conversion.tpp>
#undef __CML_MATHLIB_COORDINATE_CONVERSION_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
