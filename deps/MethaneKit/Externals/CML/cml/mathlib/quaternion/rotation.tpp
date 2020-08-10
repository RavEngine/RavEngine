/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATHLIB_QUATERNION_ROTATION_TPP
#error "mathlib/quaternion/rotation.tpp not included correctly"
#endif

#include <cml/common/mpl/are_convertible.h>
#include <cml/scalar/traits.h>
#include <cml/vector/size_checking.h>
#include <cml/matrix/fixed_compiled.h>
#include <cml/mathlib/matrix/rotation.h>
#include <cml/mathlib/matrix/misc.h>

namespace cml {

/* Builders: */

template<class Sub, class E> inline void
quaternion_rotation_world_axis(
  writable_quaternion<Sub>& q, int axis, E angle
  )
{
  static_assert(cml::are_convertible<
    value_type_trait_of_t<Sub>, E>::value, "incompatible scalar types");

  typedef order_type_trait_of_t<Sub>			order_type;
  typedef traits_of_t<E>				angle_traits;

  cml_require(0 <= axis && axis <= 2, std::invalid_argument, "invalid axis");

  q.identity();
  q[order_type::W] = angle_traits::cos(angle / E(2));
  q[order_type::X + axis] = angle_traits::sin(angle / E(2));
}

template<class Sub, class E> inline void
quaternion_rotation_world_x(writable_quaternion<Sub>& q, E angle)
{
  quaternion_rotation_world_axis(q, 0, angle);
}

template<class Sub, class E> inline void
quaternion_rotation_world_y(writable_quaternion<Sub>& q, E angle)
{
  quaternion_rotation_world_axis(q, 1, angle);
}

template<class Sub, class E> inline void
quaternion_rotation_world_z(writable_quaternion<Sub>& q, E angle)
{
  quaternion_rotation_world_axis(q, 2, angle);
}


template<class Sub, class ASub, class E>
inline void quaternion_rotation_axis_angle(
  writable_quaternion<Sub>& q, const readable_vector<ASub>& axis, E angle
  )
{
  static_assert(cml::are_convertible<
    value_type_trait_of_t<Sub>, value_type_trait_of_t<ASub>, E>::value,
    "incompatible scalar types");

  typedef traits_of_t<E>				angle_traits;

  cml::check_size(axis, int_c<3>());
  q.set(
    angle_traits::cos(angle / E(2)),
    angle_traits::sin(angle / E(2)) * axis);
}

template<class Sub, class MSub>
inline void quaternion_rotation_matrix(
  writable_quaternion<Sub>& q, const readable_matrix<MSub>& m
  )
{
  static_assert(cml::are_convertible<
    value_type_trait_of_t<Sub>, value_type_trait_of_t<MSub>>::value,
    "incompatible scalar types");

  typedef order_type_trait_of_t<Sub>			order_type;
  typedef value_type_trait_of_t<Sub>			value_type;
  typedef traits_of_t<value_type>			value_traits;
  typedef value_type_trait_of_t<MSub>			M_type;
  typedef traits_of_t<M_type>				M_traits;

  cml::check_minimum_size(m, int_c<3>(), int_c<3>());

  /* Local version of the quaternion ordering: */
  enum {
    W = order_type::W,
    X = order_type::X,
    Y = order_type::Y,
    Z = order_type::Z
  };

  auto tr = trace_3x3(m);
  if(tr >= value_type(0)) {
    q[W] = value_traits::sqrt(tr + value_type(1)) / value_type(2);
    value_type s = (value_type(1) / value_type(4)) / q[W];
    q[X] = (m.basis_element(1,2) - m.basis_element(2,1)) * s;
    q[Y] = (m.basis_element(2,0) - m.basis_element(0,2)) * s;
    q[Z] = (m.basis_element(0,1) - m.basis_element(1,0)) * s;
  } else {
    int largest_diagonal_element = index_of_max(
      m.basis_element(0,0), m.basis_element(1,1), m.basis_element(2,2));
    int i, j, k;
    cyclic_permutation(largest_diagonal_element, i, j, k);
    const int I = X + i;
    const int J = X + j;
    const int K = X + k;
    q[I] = value_type(M_traits::sqrt(
      m.basis_element(i,i) - m.basis_element(j,j)
      - m.basis_element(k,k) + value_type(1))) / value_type(2);
    value_type s = (value_type(1) / value_type(4)) / q[I];
    q[J] = (m.basis_element(i,j) + m.basis_element(j,i)) * s;
    q[K] = (m.basis_element(i,k) + m.basis_element(k,i)) * s;
    q[W] = (m.basis_element(j,k) - m.basis_element(k,j)) * s;
  }
}

template<class Sub, class E0, class E1, class E2> void
quaternion_rotation_euler(writable_quaternion<Sub>& q,
  E0 angle_0, E1 angle_1, E2 angle_2, euler_order order
  )
{
  static_assert(cml::are_convertible<
    value_type_trait_of_t<Sub>, E0, E1, E2>::value,
    "incompatible scalar types");

  typedef order_type_trait_of_t<Sub>			order_type;
  typedef scalar_traits<E0>				angle0_traits;
  typedef scalar_traits<E1>				angle1_traits;
  typedef scalar_traits<E2>				angle2_traits;

  int i, j, k;
  bool odd, repeat;
  cml::unpack_euler_order(order, i, j, k, odd, repeat);

  const int W = order_type::W;
  const int I = order_type::X + i;
  const int J = order_type::X + j;
  const int K = order_type::X + k;

  if(odd) angle_1 = -angle_1;

  angle_0 /= E0(2);
  angle_1 /= E1(2);
  angle_2 /= E2(2);

  auto s0 = angle0_traits::sin(angle_0);
  auto c0 = angle0_traits::cos(angle_0);
  auto s1 = angle1_traits::sin(angle_1);
  auto c1 = angle1_traits::cos(angle_1);
  auto s2 = angle2_traits::sin(angle_2);
  auto c2 = angle2_traits::cos(angle_2);

  auto s0s2 = s0 * s2;
  auto s0c2 = s0 * c2;
  auto c0s2 = c0 * s2;
  auto c0c2 = c0 * c2;

  if(repeat) {
    q[I] = c1 * (c0s2 + s0c2);
    q[J] = s1 * (c0c2 + s0s2);
    q[K] = s1 * (c0s2 - s0c2);
    q[W] = c1 * (c0c2 - s0s2);
  } else {
    q[I] = c1 * s0c2 - s1 * c0s2;
    q[J] = c1 * s0s2 + s1 * c0c2;
    q[K] = c1 * c0s2 - s1 * s0c2;
    q[W] = c1 * c0c2 + s1 * s0s2;
  }

  if(odd) q[J] = -q[J];
}

template<class Sub, class ESub> void
quaternion_rotation_euler(
  writable_quaternion<Sub>& q,
  const readable_vector<ESub>& euler, euler_order order
  )
{
  cml::check_size(euler, cml::int_c<3>());
  quaternion_rotation_euler(q, euler[0], euler[1], euler[2], order);
}


/* Alignment: */

template<class Sub, class ASub, class RSub> inline void
quaternion_rotation_align(
  writable_quaternion<Sub>& q,
  const readable_vector<ASub>& align, const readable_vector<RSub>& reference,
  bool normalize, axis_order order
  )
{
  typedef value_type_trait_of_t<Sub>			value_type;
  typedef matrix<value_type, compiled<3,3>>		temporary_type;

  temporary_type m;
  matrix_rotation_align(m, align, reference, normalize, order);
  quaternion_rotation_matrix(q, m);
}

template<class Sub, class PSub, class TSub, class RSub> void
quaternion_rotation_aim_at(
  writable_quaternion<Sub>& q,
  const readable_vector<PSub>& pos, const readable_vector<TSub>& target,
  const readable_vector<RSub>& reference,
  axis_order order
  )
{
  typedef value_type_trait_of_t<Sub>			value_type;
  typedef matrix<value_type, compiled<3,3>>		temporary_type;

  temporary_type m;
  matrix_rotation_aim_at(m, pos, target, reference, order);
  quaternion_rotation_matrix(q, m);
}


/* Conversion: */

template<class Sub, class ASub, class E, class Tol> void
quaternion_to_axis_angle(
  const readable_quaternion<Sub>& q,
  writable_vector<ASub>& axis, E& angle,
  Tol tolerance
  )
{
  static_assert(cml::are_convertible<
    value_type_trait_of_t<Sub>, value_type_trait_of_t<ASub>, E, Tol>::value,
    "incompatible scalar types");

  typedef scalar_traits<E>				angle_traits;

  cml::detail::check_or_resize(axis, int_c<3>());

  axis = q.imaginary();
  auto l = axis.length();
  if(l > tolerance) {
    axis /= l;
    angle = E(2) * angle_traits::atan2(l, q.real());
  } else {
    axis.zero();
    angle = E(0);
  }
}

namespace detail {

/** Helper for the quaternion_to_axis_angle() overloads. */
template<class VectorT, class Sub, class Tol> 
inline std::tuple<VectorT, value_type_trait_of_t<Sub>>
quaternion_to_axis_angle(const readable_quaternion<Sub>& q, Tol tolerance)
{
  static_assert(cml::are_convertible<
    value_type_trait_of_t<Sub>, value_type_trait_of_t<VectorT>, Tol>::value,
    "incompatible scalar types");

  VectorT axis; cml::detail::check_or_resize(axis, int_c<3>());
  value_type_trait_of_t<Sub> angle;
  cml::quaternion_to_axis_angle(q, axis, angle, tolerance);
  return std::make_tuple(axis, angle);
}

} // namespace detail

template<class Sub, class Tol>
inline std::tuple<
 vector<value_type_trait_of_t<Sub>, compiled<3>>, value_type_trait_of_t<Sub>
 >
quaternion_to_axis_angle(
  const readable_quaternion<Sub>& q, Tol tolerance
  )
{
  typedef vector<value_type_trait_of_t<Sub>, compiled<3>> vector_type;
  return detail::quaternion_to_axis_angle<vector_type, Sub, Tol>(q, tolerance);
}

template<class Sub, class E0, class E1, class E2, class Tol>
inline void
quaternion_to_euler(
  const readable_quaternion<Sub>& q,
  E0& angle_0, E1& angle_1, E2& angle_2, euler_order order,
  Tol tolerance,
  enable_if_quaternion_t<Sub>*
  )
{
  static_assert(cml::are_convertible<
    value_type_trait_of_t<Sub>, E0, E1, E2, Tol>::value,
    "incompatible scalar types");

  typedef value_type_trait_of_t<Sub>			value_type;
  typedef matrix<value_type, compiled<3,3>>		temporary_type;

  temporary_type m;
  matrix_rotation_quaternion(m, q);
  matrix_to_euler(m, angle_0, angle_1, angle_2, order, tolerance);
}

template<class Sub, class Tol>
inline vector<value_type_trait_of_t<Sub>, compiled<3>>
quaternion_to_euler(
  const readable_quaternion<Sub>& q,
  euler_order order,
  Tol tolerance, enable_if_quaternion_t<Sub>*
  )
{
  typedef value_type_trait_of_t<Sub>			value_type;
  typedef matrix<value_type, compiled<3,3>>		temporary_type;

  temporary_type m;
  matrix_rotation_quaternion(m, q);
  return matrix_to_euler(m, order, tolerance);
}

template<class VectorT, class Sub, class Tol>
inline VectorT
quaternion_to_euler(
  const readable_quaternion<Sub>& q, euler_order order,
  Tol tolerance,
  enable_if_vector_t<VectorT>*, enable_if_quaternion_t<Sub>*
  )
{
  typedef value_type_trait_of_t<Sub>			value_type;
  typedef matrix<value_type, compiled<3,3>>		temporary_type;

  temporary_type m;
  matrix_rotation_quaternion(m, q);
  return matrix_to_euler<VectorT>(m, order, tolerance);
}

} // namespace cml

#if 0
// XXX INCOMPLETE XXX

//////////////////////////////////////////////////////////////////////////////
// Rotation to align with a vector, multiple vectors, or the view plane
//////////////////////////////////////////////////////////////////////////////

/** See vector_ortho.h for details */
template < typename E, class A, class O, class C, class VecT > void
quaternion_rotation_align(quaternion<E,A,O,C>& q, const VecT& align,
    bool normalize = true, axis_order order = axis_order_zyx)
{
    typedef matrix< E,fixed<3,3>,row_basis,row_major > matrix_type;
    
    matrix_type m;
    matrix_rotation_align(m,align,normalize,order);
    quaternion_rotation_matrix(q,m);
}

/** See vector_ortho.h for details */
template < typename E,class A,class O,class C,class VecT_1,class VecT_2 > void
quaternion_rotation_align_axial(quaternion<E,A,O,C>& q, const VecT_1& align,
    const VecT_2& axis, bool normalize = true,
    axis_order order = axis_order_zyx)
{
    typedef matrix< E,fixed<3,3>,row_basis,row_major > matrix_type;
    
    matrix_type m;
    matrix_rotation_align_axial(m,align,axis,normalize,order);
    quaternion_rotation_matrix(q,m);
}

/** See vector_ortho.h for details */
template < typename E, class A, class O, class C, class MatT > void
quaternion_rotation_align_viewplane(
    quaternion<E,A,O,C>& q,
    const MatT& view_matrix,
    Handedness handedness,
    axis_order order = axis_order_zyx)
{
    typedef matrix< E,fixed<3,3>,row_basis,row_major > matrix_type;
    
    matrix_type m;
    matrix_rotation_align_viewplane(m,view_matrix,handedness,order);
    quaternion_rotation_matrix(q,m);
}

/** See vector_ortho.h for details */
template < typename E, class A, class O, class C, class MatT > void
quaternion_rotation_align_viewplane_LH(
    quaternion<E,A,O,C>& q,
    const MatT& view_matrix,
    axis_order order = axis_order_zyx)
{
    typedef matrix< E,fixed<3,3>,row_basis,row_major > matrix_type;
    
    matrix_type m;
    matrix_rotation_align_viewplane_LH(m,view_matrix,order);
    quaternion_rotation_matrix(q,m);
}

/** See vector_ortho.h for details */
template < typename E, class A, class O, class C, class MatT > void
quaternion_rotation_align_viewplane_RH(
    quaternion<E,A,O,C>& q,
    const MatT& view_matrix,
    axis_order order = axis_order_zyx)
{
    typedef matrix< E,fixed<3,3>,row_basis,row_major > matrix_type;
    
    matrix_type m;
    matrix_rotation_align_viewplane_RH(m,view_matrix,order);
    quaternion_rotation_matrix(q,m);
}

//////////////////////////////////////////////////////////////////////////////
// Rotation to aim at a target
//////////////////////////////////////////////////////////////////////////////

/** See vector_ortho.h for details */
template < typename E, class A, class O, class C,
    class VecT_1, class VecT_2 > void
quaternion_rotation_aim_at(
    quaternion<E,A,O,C>& q,
    const VecT_1& pos,
    const VecT_2& target,
    axis_order order = axis_order_zyx)
{
    typedef matrix< E,fixed<3,3>,row_basis,row_major > matrix_type;
    
    matrix_type m;
    matrix_rotation_aim_at(m,pos,target,order);
    quaternion_rotation_matrix(q,m);
}

/** See vector_ortho.h for details */
template < typename E, class A, class O, class C,
    class VecT_1, class VecT_2, class VecT_3 > void
quaternion_rotation_aim_at_axial(
    quaternion<E,A,O,C>& q,
    const VecT_1& pos,
    const VecT_2& target,
    const VecT_3& axis,
    axis_order order = axis_order_zyx)
{
    typedef matrix< E,fixed<3,3>,row_basis,row_major > matrix_type;
    
    matrix_type m;
    matrix_rotation_aim_at_axial(m,pos,target,axis,order);
    quaternion_rotation_matrix(q,m);
}

//////////////////////////////////////////////////////////////////////////////
// Relative rotation about world axes
//////////////////////////////////////////////////////////////////////////////

/* Rotate a quaternion about the given world axis */
template < class E, class A, class O, class C > void
quaternion_rotate_about_world_axis(quaternion<E,A,O,C>& q,size_t axis, E angle)
{
    typedef quaternion<E,A,O,C> quaternion_type;
    typedef typename quaternion_type::value_type value_type;
    typedef typename quaternion_type::order_type order_type;

    /* Checking */
    detail::CheckIndex3(axis);

    size_t i, j, k;
    cyclic_permutation(axis, i, j, k);
    
    const size_t W = order_type::W;
    const size_t I = order_type::X + i;
    const size_t J = order_type::X + j;
    const size_t K = order_type::X + k;
    
    angle *= value_type(.5);
    value_type s = value_type(std::sin(angle));
    value_type c = value_type(std::cos(angle));

    quaternion_type result;
    result[I] = c * q[I] + s * q[W];
    result[J] = c * q[J] - s * q[K];
    result[K] = c * q[K] + s * q[J];
    result[W] = c * q[W] - s * q[I];
    q = result;
}

/* Rotate a quaternion about the world x axis */
template < class E, class A, class O, class C > void
quaternion_rotate_about_world_x(quaternion<E,A,O,C>& q, E angle) {
    quaternion_rotate_about_world_axis(q,0,angle);
}

/* Rotate a quaternion about the world y axis */
template < class E, class A, class O, class C > void
quaternion_rotate_about_world_y(quaternion<E,A,O,C>& q, E angle) {
    quaternion_rotate_about_world_axis(q,1,angle);
}

/* Rotate a quaternion about the world z axis */
template < class E, class A, class O, class C > void
quaternion_rotate_about_world_z(quaternion<E,A,O,C>& q, E angle) {
    quaternion_rotate_about_world_axis(q,2,angle);
}

//////////////////////////////////////////////////////////////////////////////
// Relative rotation about local axes
//////////////////////////////////////////////////////////////////////////////

/* Rotate a quaternion about the given local axis */
template < class E, class A, class O, class C > void
quaternion_rotate_about_local_axis(quaternion<E,A,O,C>& q,size_t axis, E angle)
{
    typedef quaternion<E,A,O,C> quaternion_type;
    typedef typename quaternion_type::value_type value_type;
    typedef typename quaternion_type::order_type order_type;

    /* Checking */
    detail::CheckIndex3(axis);

    size_t i, j, k;
    cyclic_permutation(axis, i, j, k);
    
    const size_t W = order_type::W;
    const size_t I = order_type::X + i;
    const size_t J = order_type::X + j;
    const size_t K = order_type::X + k;
    
    angle *= value_type(.5);
    value_type s = value_type(std::sin(angle));
    value_type c = value_type(std::cos(angle));

    quaternion_type result;
    result[I] = c * q[I] + s * q[W];
    result[J] = c * q[J] + s * q[K];
    result[K] = c * q[K] - s * q[J];
    result[W] = c * q[W] - s * q[I];
    q = result;
}

/* Rotate a quaternion about its local x axis */
template < class E, class A, class O, class C > void
quaternion_rotate_about_local_x(quaternion<E,A,O,C>& q, E angle) {
    quaternion_rotate_about_local_axis(q,0,angle);
}

/* Rotate a quaternion about its local y axis */
template < class E, class A, class O, class C > void
quaternion_rotate_about_local_y(quaternion<E,A,O,C>& q, E angle) {
    quaternion_rotate_about_local_axis(q,1,angle);
}

/* Rotate a quaternion about its local z axis */
template < class E, class A, class O, class C > void
quaternion_rotate_about_local_z(quaternion<E,A,O,C>& q, E angle) {
    quaternion_rotate_about_local_axis(q,2,angle);
}

//////////////////////////////////////////////////////////////////////////////
// Rotation from vector to vector
//////////////////////////////////////////////////////////////////////////////

/* http://www.martinb.com/maths/algebra/vectors/angleBetween/index.htm. */

/** Build a quaternion to rotate from one vector to another */
template < class E,class A,class O,class C,class VecT_1,class VecT_2 > void
quaternion_rotation_vec_to_vec(
    quaternion<E,A,O,C>& q,
    const VecT_1& v1,
    const VecT_2& v2,
    bool unit_length_vectors = false)
{
    typedef quaternion<E,A,O,C> quaternion_type;
    typedef typename quaternion_type::value_type value_type;
    typedef vector< value_type, fixed<3> > vector_type;
    
    /* Checking handled by cross() */

    /* @todo: If at some point quaternion<> has a set() function that takes a
     * vector and a scalar, this can then be written as:
     *
     * if (...) {
     *     q.set(value_type(1)+dot(v1,v2), cross(v1,v2));
     * } else {
     *     q.set(std::sqrt(...)+dot(v1,v2), cross(v1,v2));
     * }
     */
     
    vector_type c = cross(v1,v2);
    if (unit_length_vectors) {
        q = quaternion_type(value_type(1) + dot(v1,v2), c.data());
    } else {
        q = quaternion_type(
            std::sqrt(v1.length_squared() * v2.length_squared()) + dot(v1,v2),
            c/*.data()*/
        );
    }
    q.normalize();
}

//////////////////////////////////////////////////////////////////////////////
// Scale the angle of a rotation matrix
//////////////////////////////////////////////////////////////////////////////

template < typename E, class A, class O, class C > void
quaternion_scale_angle(quaternion<E,A,O,C>& q, E t,
    E tolerance = epsilon<E>::placeholder())
{
    typedef vector< E,fixed<3> > vector_type;
    typedef typename vector_type::value_type value_type;
    
    vector_type axis;
    value_type angle;
    quaternion_to_axis_angle(q, axis, angle, tolerance);
    quaternion_rotation_axis_angle(q, axis, angle * t);
}

//////////////////////////////////////////////////////////////////////////////
// Support functions for uniform handling of pos- and neg-cross quaternions
//////////////////////////////////////////////////////////////////////////////

namespace detail {

/** Concatenate two quaternions in the order q1->q2 */
template < class QuatT_1, class QuatT_2 >
typename et::QuaternionPromote2<QuatT_1,QuatT_2>::temporary_type
quaternion_rotation_difference(
    const QuatT_1& q1, const QuatT_2& q2, positive_cross)
{
    return q2 * conjugate(q1);
}

/** Concatenate two quaternions in the order q1->q2 */
template < class QuatT_1, class QuatT_2 >
typename et::QuaternionPromote2<QuatT_1,QuatT_2>::temporary_type
quaternion_rotation_difference(
    const QuatT_1& q1, const QuatT_2& q2, negative_cross)
{
    return conjugate(q1) * q2;
}

} // namespace detail

//////////////////////////////////////////////////////////////////////////////
// Quaternions rotation difference
//////////////////////////////////////////////////////////////////////////////

/** Return the rotational 'difference' between two quaternions */
template < class QuatT_1, class QuatT_2 >
typename et::QuaternionPromote2<QuatT_1,QuatT_2>::temporary_type
quaternion_rotation_difference(const QuatT_1& q1, const QuatT_2& q2) {
    return detail::quaternion_rotation_difference(
        q1, q2, typename QuatT_1::cross_type());
}

//////////////////////////////////////////////////////////////////////////////
// Conversions
//////////////////////////////////////////////////////////////////////////////

/** Convert a quaternion to an axis-angle pair */
template < class QuatT, typename E, class A > void
quaternion_to_axis_angle(
    const QuatT& q,
    vector<E,A>& axis,
    E& angle,
    E tolerance = epsilon<E>::placeholder())
{
    typedef QuatT quaternion_type;
    typedef typename quaternion_type::value_type value_type;
    typedef typename quaternion_type::order_type order_type;
    
    /* Checking */
    detail::CheckQuat(q);

    axis = q.imaginary();
    value_type l = length(axis);
    if (l > tolerance) {
        axis /= l;
        angle = value_type(2) * std::atan2(l,q.real());
    } else {
        axis.zero();
        angle = value_type(0);
    }
}

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
