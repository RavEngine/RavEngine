/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_quaternion_rotation_h
#define	cml_mathlib_quaternion_rotation_h

#include <cml/vector/fwd.h>
#include <cml/matrix/fwd.h>
#include <cml/quaternion/writable_quaternion.h>
#include <cml/mathlib/euler_order.h>
#include <cml/mathlib/axis_order.h>

/** @defgroup mathlib_quaternion_rotation Quaternion Rotation Functions
 *
 * @note A number of these functions simply wrap calls to the corresponding
 * matrix functions. Some of them (the 'aim-at' and 'align' functions in
 * particular) might be considered a bit superfluous, since the resulting
 * quaternion will most likely be converted to a matrix at some point
 * anyway.  However, they're included here for completeness, and for
 * convenience in cases where a quaternion is being used as the primary
 * rotation representation.
 */

namespace cml {

/** @addtogroup mathlib_quaternion_rotation */
/*@{*/

/** @defgroup mathlib_quaternion_rotation_builders Quaternion Rotation Builders  */
/*@{*/

/** Build a quaternion representing a rotation about world axis @c axis.
 *
 * @throws std::invalid_argument if @c axis < 0 or @c axis > 2.
 */
template<class Sub, class E> void
quaternion_rotation_world_axis(writable_quaternion<Sub>& q, int axis, E angle);

/** Build a quaternion representing a rotation about world x-axis. */
template<class Sub, class E> void
quaternion_rotation_world_x(writable_quaternion<Sub>& q, E angle);

/** Build a quaternion representing a rotation about world y-axis. */
template<class Sub, class E> void
quaternion_rotation_world_y(writable_quaternion<Sub>& q, E angle);

/** Build a quaternion representing a rotation about world z-axis. */
template<class Sub, class E> void
quaternion_rotation_world_z(writable_quaternion<Sub>& q, E angle);

/** Build a quaternion from an axis-angle pair.
 *
 * @throws vector_size_error at run-time if @c axis is dynamically-sized,
 * and is not 3D.  If @c axis is fixed-size, the size is checked at
 * compile-time.
 */
template<class Sub, class ASub, class E> void
quaternion_rotation_axis_angle(
  writable_quaternion<Sub>& q, const readable_vector<ASub>& axis, E angle);

/** Build a quaternion from a rotation matrix.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 3x3.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class MSub> void
quaternion_rotation_matrix(
  writable_quaternion<Sub>& q, const readable_matrix<MSub>& m);

/** Compute a quaternion given three Euler angles and the required
 * order.
 *
 * The rotations are applied about the cardinal axes in the order specified
 * by the 'order' argument, where 'order' is one of the following
 * enumerants:
 *
 * euler_order_xyz
 * euler_order_xzy
 * euler_order_xyx
 * euler_order_xzx
 * euler_order_yzx
 * euler_order_yxz
 * euler_order_yzy
 * euler_order_yxy
 * euler_order_zxy
 * euler_order_zyx
 * euler_order_zxz
 * euler_order_zyz
 *
 * e.g. euler_order_xyz means compute the quaternion equivalent to R_x *
 * R_y * R_z, where R_i is the rotation matrix above axis i (the row-basis
 * matrix would be R_z * R_y * R_x).
 */
template<class Sub, class E0, class E1, class E2> void
quaternion_rotation_euler(writable_quaternion<Sub>& q,
  E0 angle_0, E1 angle_1, E2 angle_2, euler_order order);

/** Compute a quaternion given a vector containing the Euler angles.
 *
 * @throws vector_size_error at run-time if @c euler is dynamically-sized,
 * and is not 3D.  If fixed-size, the sizs is checked at compile-time.
 */
template<class Sub, class ESub> void
quaternion_rotation_euler(writable_quaternion<Sub>& q,
  const readable_vector<ESub>& euler, euler_order order);

/*@}*/


/** @defgroup mathlib_quaternion_rotation_alignment Quaternion Alignment  */
/*@{*/

/** Compute a quaternion that aligns vector @c align to @c reference,
 * using rotations in axis order @c order.
 *
 * @note This uses matrix_rotation_align internally.
 */
template<class Sub, class ASub, class RSub> void
quaternion_rotation_align(writable_quaternion<Sub>& q,
  const readable_vector<ASub>& align, const readable_vector<RSub>& reference,
  bool normalize = true, axis_order order = axis_order_zyx);

/** Compute a quaternion to align the vector from @c pos to @c target
 * with @c reference.
 *
 * @note This uses matrix_rotation_aim_at internally.
 */
template<class Sub, class PSub, class TSub, class RSub> void
quaternion_rotation_aim_at(writable_quaternion<Sub>& q,
  const readable_vector<PSub>& pos, const readable_vector<TSub>& target,
  const readable_vector<RSub>& reference, axis_order order = axis_order_zyx);

/*@}*/

/** @defgroup mathlib_quaternion_rotation_conversion Quaternion Conversion  */
/*@{*/

/** Convert a quaternion @c q to an axis-angle pair.
 *
 * @note @c tolerance is used to detect a near-zero axis length.
 */
template<class Sub, class ASub, class E,
  class Tol = value_type_trait_of_t<Sub>> void
quaternion_to_axis_angle(
  const readable_quaternion<Sub>& q, writable_vector<ASub>& axis,
  E& angle, Tol tolerance = CML_DUMMY_TYPENAME scalar_traits<Tol>::epsilon());

/** Convert a quaternion @c q to an axis-angle pair returned as a
 * std::tuple.
 *
 * @note @c tolerance is used to detect a near-zero axis length.
 */
template<class Sub, class Tol = value_type_trait_of_t<Sub>>
std::tuple<
 vector<value_type_trait_of_t<Sub>, compiled<3>>, value_type_trait_of_t<Sub>
 >
quaternion_to_axis_angle(
  const readable_quaternion<Sub>& q,
  Tol tolerance = CML_DUMMY_TYPENAME scalar_traits<Tol>::epsilon());

/** Convert a quaternion @c q to an Euler-angle triple.
 *
 * @note @c tolerance is used to detect degeneracies.
 */
template<class Sub, class E0, class E1, class E2,
  class Tol = value_type_trait_of_t<Sub>> void
quaternion_to_euler(
  const readable_quaternion<Sub>& q, E0& angle_0, E1& angle_1, E2& angle_2,
  euler_order order, Tol tolerance = CML_DUMMY_TYPENAME scalar_traits<Tol>::epsilon(),
  enable_if_quaternion_t<Sub>* = nullptr);

/** Convert a quaternion @c q to an Euler-angle triple, and return
 * the result as a fixed-size 3D vector.
 *
 * @note @c tolerance is used to detect degeneracies.
 */
template<class Sub, class Tol = value_type_trait_of_t<Sub>>
vector<value_type_trait_of_t<Sub>, compiled<3>>
quaternion_to_euler(
  const readable_quaternion<Sub>& q, euler_order order,
  Tol tolerance = CML_DUMMY_TYPENAME scalar_traits<Tol>::epsilon(),
  enable_if_quaternion_t<Sub>* = nullptr);

/** Convert a quaternion @c q to an Euler-angle triple, and return
 * the result as a user-specified vector type.
 *
 * @note @c tolerance is used to detect degeneracies.
 *
 * @note @c VectorT can be any vector type with 3 elements, having internal
 * storage (e.g.  compiled<3> or allocated<>).  The default is vector<T,
 * compiled<3>>, where T is the value_type of the matrix.
 */
template<class VectorT, class Sub,
  class Tol = value_type_trait_of_t<Sub>> VectorT
quaternion_to_euler(
  const readable_quaternion<Sub>& q, euler_order order,
  Tol tolerance = CML_DUMMY_TYPENAME scalar_traits<Tol>::epsilon(),
  enable_if_vector_t<VectorT>* = nullptr,
  enable_if_quaternion_t<Sub>* = nullptr);

/*@}*/

/*@}*/

} // namespace cml

#define __CML_MATHLIB_QUATERNION_ROTATION_TPP
#include <cml/mathlib/quaternion/rotation.tpp>
#undef __CML_MATHLIB_QUATERNION_ROTATION_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
