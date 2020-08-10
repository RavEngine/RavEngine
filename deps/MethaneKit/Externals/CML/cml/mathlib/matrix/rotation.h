/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_matrix_rotation_h
#define	cml_mathlib_matrix_rotation_h

#include <tuple>
#include <cml/scalar/traits.h>
#include <cml/storage/compiled_selector.h>
#include <cml/vector/vector.h>
#include <cml/vector/type_util.h>
#include <cml/matrix/type_util.h>
#include <cml/quaternion/fwd.h>
#include <cml/mathlib/euler_order.h>
#include <cml/mathlib/axis_order.h>

/** @defgroup mathlib_matrix_rotation Matrix Rotation Functions */

namespace cml {

/** @addtogroup mathlib_matrix_rotation */
/*@{*/

/** @defgroup mathlib_matrix_rotation_builders_2D 2D Rotation Matrix Builders  */
/*@{*/

/** Compute a 2D rotation matrix give @c angle.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 2x2.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_rotation_2D(writable_matrix<Sub>& m, E angle);

/*@}*/

/** @defgroup mathlib_matrix_rotation_alignment_2D 2D Rotation Alignment  */
/*@{*/

/** Compute a rotation matrix that aligns the x- or y-axis to @c align,
 * based on the axis order @c order.
 * 
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 2x2.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class ASub> void
matrix_rotation_align_2D(
  writable_matrix<Sub>& m, const readable_vector<ASub>& align,
  bool normalize = true, axis_order2D order = axis_order_xy);

/*@}*/


/** @defgroup mathlib_matrix_rotation_builders_3D 3D Rotation Matrix Builders  */
/*@{*/

/** Compute a matrix representing a 3D rotation @c angle about world axis
 * @c axis.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 3x3.  If @c m is fixed-size, the
 * size is checked at compile-time.
 *
 * @throws std::invalid_argument if @c axis < 0 or @c axis > 2.
 */
template<class Sub, class E> void
matrix_rotation_world_axis(writable_matrix<Sub>& m, int axis, const E& angle);

/** Compute a matrix representing a 3D rotation @c angle about the world
 * x-axis.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 3x3.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_rotation_world_x(writable_matrix<Sub>& m, const E& angle);

/** Compute a matrix representing a 3D rotation @c angle about the world
 * y-axis.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 3x3.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_rotation_world_y(writable_matrix<Sub>& m, const E& angle);

/** Compute a matrix representing a 3D rotation @c angle about the world
 * z-axis.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 3x3.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E> void
matrix_rotation_world_z(writable_matrix<Sub>& m, const E& angle);

/** Compute a rotation matrix from an axis and angle.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 3x3.  If @c m is fixed-size, the
 * size is checked at compile-time.
 *
 * @throws vector_size_error at run-time if @c v is dynamically-sized, and
 * is not 3D.  If @c v is fixed-size, the size is checked at compile-time.
 */
template<class Sub, class QSub, class E> void matrix_rotation_axis_angle(
  writable_matrix<Sub>& m, const readable_vector<QSub>& axis, const E& angle);

/** Compute a rotation matrix given three Euler angles and the required
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
 * e.g. euler_order_xyz means compute the column-basis rotation matrix
 * equivalent to R_x * R_y * R_z, where R_i is the rotation matrix above
 * axis i (the row-basis matrix would be R_z * R_y * R_x).
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 3x3.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class E0, class E1, class E2> void
matrix_rotation_euler(writable_matrix<Sub>& m,
  E0 angle_0, E1 angle_1, E2 angle_2, euler_order order);

/** Compute a rotation matrix given a vector containing the Euler angles.
 *
 * @throws vector_size_error at run-time if @c euler is dynamically-sized,
 * and is not 3D.  If fixed-size, the sizs is checked at compile-time.
 */
template<class Sub, class ESub> void
matrix_rotation_euler(writable_matrix<Sub>& m,
  const readable_vector<ESub>& euler, euler_order order);

/** Build a matrix of derivatives of Euler angles about the specified axis.
 *
 * The rotation derivatives are applied about the cardinal axes in the
 * order specified by the 'order' argument, where 'order' is one of the
 * following enumerants:
 *
 * euler_order_xyz
 * euler_order_xzy
 * euler_order_yzx
 * euler_order_yxz
 * euler_order_zxy
 * euler_order_zyx
 *
 * e.g. euler_order_xyz means compute the column-basis rotation matrix
 * equivalent to R_x * R_y * R_z, where R_i is the rotation matrix above
 * axis i (the row-basis matrix would be R_z * R_y * R_x).
 *
 * The derivative is taken with respect to the specified 'axis', which is
 * the position of the axis in the triple; e.g. if order = euler_order_xyz,
 * then axis = 0 would mean take the derivative with respect to x.  Note
 * that repeated axes are not currently supported.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 3x3.  If @c m is fixed-size, the
 * size is checked at compile-time.
 *
 * @throws std::invalid_argument if @c axis is not 0, 1, or 2, or if @c
 * order has a repeated axis.
 */
template<class Sub, class E0, class E1, class E2> void
matrix_rotation_euler_derivatives(writable_matrix<Sub>& m, int axis,
  E0 angle_0, E1 angle_1, E2 angle_2, euler_order order);

/** Build a matrix of derivatives of Euler angles about the specified axis,
 * given the angles as a vector.
 *
 * @throws vector_size_error at run-time if @c euler is dynamically-sized,
 * and is not 3D.  If fixed-size, the sizs is checked at compile-time.
 */
template<class Sub, class ESub> void
matrix_rotation_euler_derivatives(writable_matrix<Sub>& m, int axis,
  const readable_vector<ESub>& euler, euler_order order);

/** Compute a rotation matrix from a quaternion.
 *
 * @throws minimum_matrix_size_error at run-time if @c m is
 * dynamically-sized, and is not at least 3x3.  If @c m is fixed-size, the
 * size is checked at compile-time.
 */
template<class Sub, class QSub> void
matrix_rotation_quaternion(
  writable_matrix<Sub>& m, const readable_quaternion<QSub>& q);

/*@}*/

/** @defgroup mathlib_matrix_rotation_alignment 3D Rotation Alignment  */
/*@{*/

/** Compute a rotation matrix that aligns vector @c align to @c reference,
 * using rotations in axis order @c order.
 */
template<class Sub, class ASub, class RSub> void
matrix_rotation_align(writable_matrix<Sub>& m,
  const readable_vector<ASub>& align, const readable_vector<RSub>& reference,
  bool normalize = true, axis_order order = axis_order_zyx);

/** Compute a rotation matrix to align the vector from @c pos to @c target
 * with @c reference.
 */
template<class Sub, class PSub, class TSub, class RSub> void
matrix_rotation_aim_at(writable_matrix<Sub>& m,
  const readable_vector<PSub>& pos, const readable_vector<TSub>& target,
  const readable_vector<RSub>& reference, axis_order order = axis_order_zyx);

/*@}*/

/** @defgroup mathlib_matrix_rotation_conversion Rotation Matrix Conversion  */
/*@{*/

/** Convert a 3D rotation matrix @c m to an axis-angle pair.
 *
 * @note @c tolerance is used to detect a near-zero axis length.
 */
template<class Sub, class ASub, class E,
  class Tol = value_type_trait_of_t<ASub>> void
matrix_to_axis_angle(
  const readable_matrix<Sub>& m, writable_vector<ASub>& axis,
  E& angle, Tol tolerance = CML_DUMMY_TYPENAME scalar_traits<Tol>::epsilon());

/** Convert a 3D rotation matrix @c m to an axis-angle pair returned as a
 * std::tuple.
 *
 * @note @c tolerance is used to detect a near-zero axis length.
 */
template<class Sub, class Tol = value_type_trait_of_t<Sub>>
std::tuple<
 vector<value_type_trait_of_t<Sub>, compiled<3>>, value_type_trait_of_t<Sub>
 >
matrix_to_axis_angle(
  const readable_matrix<Sub>& m,
  Tol tolerance = CML_DUMMY_TYPENAME scalar_traits<Tol>::epsilon());

/** Convert a 3D rotation matrix @c m to an Euler-angle triple.
 *
 * @note @c tolerance is used to detect degeneracies.
 */
template<class Sub, class E0, class E1, class E2,
  class Tol = value_type_trait_of_t<Sub>> void
matrix_to_euler(const readable_matrix<Sub>& m,
  E0& angle_0, E1& angle_1, E2& angle_2, euler_order order,
  Tol tolerance = CML_DUMMY_TYPENAME scalar_traits<Tol>::epsilon(),
  enable_if_matrix_t<Sub>* = nullptr);

/** Convert a 3D rotation matrix @c m to an Euler-angle triple, and return
 * the result as a fixed-size 3D vector.
 *
 * @note @c tolerance is used to detect degeneracies.
 */
template<class Sub, class Tol = value_type_trait_of_t<Sub>>
vector<value_type_trait_of_t<Sub>, compiled<3>>
matrix_to_euler(const readable_matrix<Sub>& m, euler_order order,
  Tol tolerance = CML_DUMMY_TYPENAME scalar_traits<Tol>::epsilon(),
  enable_if_matrix_t<Sub>* = nullptr);

/** Convert a 3D rotation matrix @c m to an Euler-angle triple, and return
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
matrix_to_euler(const readable_matrix<Sub>& m, euler_order order,
  Tol tolerance = CML_DUMMY_TYPENAME scalar_traits<Tol>::epsilon(),
  enable_if_vector_t<VectorT>* = nullptr, enable_if_matrix_t<Sub>* = nullptr);

/*@}*/

/*@}*/

} // namespace cml

#define __CML_MATHLIB_MATRIX_ROTATION_TPP
#include <cml/mathlib/matrix/rotation.tpp>
#undef __CML_MATHLIB_MATRIX_ROTATION_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
