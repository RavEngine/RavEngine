/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATHLIB_MATRIX_ROTATION_TPP
#error "mathlib/matrix/rotation.tpp not included correctly"
#endif

#include <cml/common/mpl/are_convertible.h>
#include <cml/scalar/functions.h>
#include <cml/vector/detail/check_or_resize.h>
#include <cml/quaternion/readable_quaternion.h>
#include <cml/mathlib/vector/orthonormal.h>
#include <cml/mathlib/matrix/basis.h>
#include <cml/mathlib/matrix/misc.h>
#include <cml/mathlib/matrix/size_checking.h>

namespace cml {

/* 2D rotations: */

template<class Sub, class E> inline void
matrix_rotation_2D(writable_matrix<Sub>& m, E angle)
{
  static_assert(
    cml::are_convertible<value_type_trait_of_t<Sub>, E>::value,
    "incompatible scalar types");

  typedef scalar_traits<E>				angle_traits;

  cml::check_linear_2D(m);

  /* Initialize: */
  m.identity();

  /* Initialize m: */
  auto s = angle_traits::sin(angle);
  auto c = angle_traits::cos(angle);
  m.set_basis_element(0,0, c);
  m.set_basis_element(0,1, s);
  m.set_basis_element(1,0,-s);
  m.set_basis_element(1,1, c);
}


/* 2D alignment: */

template<class Sub, class ASub> void
matrix_rotation_align_2D(
  writable_matrix<Sub>& m, const readable_vector<ASub>& align,
  bool normalize, axis_order2D order)
{
  static_assert(cml::are_convertible<value_type_trait_of_t<Sub>,
    value_type_trait_of_t<ASub>>::value, "incompatible scalar types");

  typedef value_type_trait_of_t<Sub>			value_type;

  cml::check_linear_2D(m);

  m.identity();

  vector<value_type, compiled<2>> x, y;
  orthonormal_basis_2D(align, x, y, normalize, order);
  matrix_set_basis_vectors_2D(m, x, y);
}


/* 3D rotations: */

template<class Sub, class E> inline void
matrix_rotation_world_axis(writable_matrix<Sub>& m, int axis, const E& angle)
{
  static_assert(cml::are_convertible<
    value_type_trait_of_t<Sub>, E>::value, "incompatible scalar types");

  typedef traits_of_t<E>				angle_traits;

  cml::check_linear_3D(m);
  cml_require(0 <= axis && axis <= 2, std::invalid_argument, "invalid axis");
 
  /* Setup sin() and cos() for the chosen axis: */
  int i, j, k; cml::cyclic_permutation(axis, i, j, k);
  auto s = angle_traits::sin(angle);
  auto c = angle_traits::cos(angle);

  /* Clear the matrix: */
  m.identity();

  /* Set elements: */
  m.set_basis_element(j,j, c);
  m.set_basis_element(j,k, s);
  m.set_basis_element(k,j,-s);
  m.set_basis_element(k,k, c);
}

template<class Sub, class E> inline void
matrix_rotation_world_x(writable_matrix<Sub>& m, const E& angle)
{
  matrix_rotation_world_axis(m, 0, angle);
}

template<class Sub, class E> inline void
matrix_rotation_world_y(writable_matrix<Sub>& m, const E& angle)
{
  matrix_rotation_world_axis(m, 1, angle);
}

template<class Sub, class E> inline void
matrix_rotation_world_z(writable_matrix<Sub>& m, const E& angle)
{
  matrix_rotation_world_axis(m, 2, angle);
}

template<class Sub, class ASub, class E> inline void
matrix_rotation_axis_angle(
  writable_matrix<Sub>& m, const readable_vector<ASub>& axis, const E& angle
  )
{
  static_assert(cml::are_convertible<
    value_type_trait_of_t<Sub>, value_type_trait_of_t<ASub>, E>::value,
    "incompatible scalar types");

  typedef scalar_traits<E>				angle_traits;

  cml::check_linear_3D(m);
  cml::check_size(axis, int_c<3>());

  /* Initialize: */
  m.identity();

  /* Precompute values: */
  auto s = angle_traits::sin(angle);
  auto c = angle_traits::cos(angle);
  auto omc = E(1) - c;

  auto xomc = axis[0] * omc;
  auto yomc = axis[1] * omc;
  auto zomc = axis[2] * omc;

  auto xxomc = axis[0] * xomc;
  auto yyomc = axis[1] * yomc;
  auto zzomc = axis[2] * zomc;
  auto xyomc = axis[0] * yomc;
  auto yzomc = axis[1] * zomc;
  auto zxomc = axis[2] * xomc;

  auto xs = axis[0] * s;
  auto ys = axis[1] * s;
  auto zs = axis[2] * s;

  m.set_basis_element(0,0, xxomc + c );
  m.set_basis_element(0,1, xyomc + zs);
  m.set_basis_element(0,2, zxomc - ys);
  m.set_basis_element(1,0, xyomc - zs);
  m.set_basis_element(1,1, yyomc + c );
  m.set_basis_element(1,2, yzomc + xs);
  m.set_basis_element(2,0, zxomc + ys);
  m.set_basis_element(2,1, yzomc - xs);
  m.set_basis_element(2,2, zzomc + c );
}

template<class Sub, class E0, class E1, class E2> inline void
matrix_rotation_euler(writable_matrix<Sub>& m,
  E0 angle_0, E1 angle_1, E2 angle_2, euler_order order
  )
{
  static_assert(cml::are_convertible<
    value_type_trait_of_t<Sub>, E0, E1, E2>::value,
    "incompatible scalar types");

  typedef scalar_traits<E0>				angle0_traits;
  typedef scalar_traits<E1>				angle1_traits;
  typedef scalar_traits<E2>				angle2_traits;

  cml::check_linear_3D(m);

  /* Initialize: */
  m.identity();

  int i, j, k;
  bool odd, repeat;
  cml::unpack_euler_order(order, i, j, k, odd, repeat);

  if(odd) {
    angle_0 = -angle_0;
    angle_1 = -angle_1;
    angle_2 = -angle_2;
  }

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

  if (repeat) {
    m.set_basis_element(i,i, c1              );
    m.set_basis_element(i,j, s1 * s2         );
    m.set_basis_element(i,k,-s1 * c2         );
    m.set_basis_element(j,i, s0 * s1         );
    m.set_basis_element(j,j,-c1 * s0s2 + c0c2);
    m.set_basis_element(j,k, c1 * s0c2 + c0s2);
    m.set_basis_element(k,i, c0 * s1         );
    m.set_basis_element(k,j,-c1 * c0s2 - s0c2);
    m.set_basis_element(k,k, c1 * c0c2 - s0s2);
  } else {
    m.set_basis_element(i,i, c1 * c2         );
    m.set_basis_element(i,j, c1 * s2         );
    m.set_basis_element(i,k,-s1              );
    m.set_basis_element(j,i, s1 * s0c2 - c0s2);
    m.set_basis_element(j,j, s1 * s0s2 + c0c2);
    m.set_basis_element(j,k, s0 * c1         );
    m.set_basis_element(k,i, s1 * c0c2 + s0s2);
    m.set_basis_element(k,j, s1 * c0s2 - s0c2);
    m.set_basis_element(k,k, c0 * c1         );
  }
}

template<class Sub, class ESub> inline void
matrix_rotation_euler(writable_matrix<Sub>& m,
  const readable_vector<ESub>& euler, euler_order order
  )
{
  cml::check_size(euler, cml::int_c<3>());
  matrix_rotation_euler(m, euler[0], euler[1], euler[2], order);
}

template<class Sub, class E0, class E1, class E2> inline void
matrix_rotation_euler_derivatives(writable_matrix<Sub>& m, int axis,
  E0 angle_0, E1 angle_1, E2 angle_2, euler_order order
  )
{
  static_assert(cml::are_convertible<
    value_type_trait_of_t<Sub>, E0, E1, E2>::value,
    "incompatible scalar types");

  typedef scalar_traits<E0>				angle0_traits;
  typedef scalar_traits<E1>				angle1_traits;
  typedef scalar_traits<E2>				angle2_traits;

  cml_require(0 <= axis && axis <= 2,
   std::invalid_argument, "axis must be 0, 1, or 2");
  cml::check_linear_3D(m);

  /* Initialize: */
  m.identity();

  int i, j, k;
  bool odd, repeat;
  cml::unpack_euler_order(order, i, j, k, odd, repeat);
  cml_require(!repeat, std::invalid_argument, "repeated axis not supported");

  if (odd) {
    angle_0 = -angle_0;
    angle_1 = -angle_1;
    angle_2 = -angle_2;
  }

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

  if(axis == 0) {
    m.set_basis_element(i,i, 0.              );
    m.set_basis_element(i,j, 0.              );
    m.set_basis_element(i,k, 0.              );
    m.set_basis_element(j,i, s1 * c0c2 + s0s2);
    m.set_basis_element(j,j, s1 * c0s2 - s0c2);
    m.set_basis_element(j,k, c0 * c1         );
    m.set_basis_element(k,i,-s1 * s0c2 + c0s2);
    m.set_basis_element(k,j,-s1 * s0s2 - c0c2);
    m.set_basis_element(k,k,-s0 * c1         );
  } else if(axis == 1) {
    m.set_basis_element(i,i,-s1 * c2         );
    m.set_basis_element(i,j,-s1 * s2         );
    m.set_basis_element(i,k,-c1              );
    m.set_basis_element(j,i, c1 * s0c2       );
    m.set_basis_element(j,j, c1 * s0s2       );
    m.set_basis_element(j,k,-s0 * s1         );
    m.set_basis_element(k,i, c1 * c0c2       );
    m.set_basis_element(k,j, c1 * c0s2       );
    m.set_basis_element(k,k,-c0 * s1         );
  } else if(axis == 2) {
    m.set_basis_element(i,i,-c1 * s2         );
    m.set_basis_element(i,j, c1 * c2         );
    m.set_basis_element(i,k, 0.              );
    m.set_basis_element(j,i,-s1 * s0s2 - c0c2);
    m.set_basis_element(j,j, s1 * s0c2 - c0s2);
    m.set_basis_element(j,k, 0.              );
    m.set_basis_element(k,i,-s1 * c0s2 + s0c2);
    m.set_basis_element(k,j, s1 * c0c2 + s0s2);
    m.set_basis_element(k,k, 0.              );
  }
}

template<class Sub, class ESub> inline void
matrix_rotation_euler_derivatives(writable_matrix<Sub>& m, int axis,
  const readable_vector<ESub>& euler, euler_order order
  )
{
  cml::check_size(euler, cml::int_c<3>());
  matrix_rotation_euler_derivatives(
    m, axis, euler[0], euler[1], euler[2], order);
}

template<class Sub, class QSub> inline void
matrix_rotation_quaternion(
  writable_matrix<Sub>& m, const readable_quaternion<QSub>& q
  )
{
  static_assert(cml::are_convertible<
    value_type_trait_of_t<Sub>, value_type_trait_of_t<QSub>>::value,
    "incompatible scalar types");

  typedef order_type_trait_of_t<QSub>			order_type;
  typedef value_type_trait_of_t<QSub>			q_type;

  cml::check_linear_3D(m);

  /* Local version of the quaternion ordering: */
  enum {
    W = order_type::W,
    X = order_type::X,
    Y = order_type::Y,
    Z = order_type::Z
  };

  auto x2 = q[X] + q[X];
  auto y2 = q[Y] + q[Y];
  auto z2 = q[Z] + q[Z];    

  auto xx2 = q[X] * x2;
  auto yy2 = q[Y] * y2;
  auto zz2 = q[Z] * z2;
  auto xy2 = q[X] * y2;
  auto yz2 = q[Y] * z2;
  auto zx2 = q[Z] * x2;
  auto xw2 = q[W] * x2;
  auto yw2 = q[W] * y2;
  auto zw2 = q[W] * z2;

  m.identity();
  m.set_basis_element(0,0, q_type(1) - yy2 - zz2);
  m.set_basis_element(0,1,             xy2 + zw2);
  m.set_basis_element(0,2,             zx2 - yw2);
  m.set_basis_element(1,0,             xy2 - zw2);
  m.set_basis_element(1,1, q_type(1) - zz2 - xx2);
  m.set_basis_element(1,2,             yz2 + xw2);
  m.set_basis_element(2,0,             zx2 + yw2);
  m.set_basis_element(2,1,             yz2 - xw2);
  m.set_basis_element(2,2, q_type(1) - xx2 - yy2);
}


/* Alignment: */

template<class Sub, class ASub, class RSub> inline void
matrix_rotation_align(
  writable_matrix<Sub>& m,
  const readable_vector<ASub>& align, const readable_vector<RSub>& reference,
  bool normalize, axis_order order
  )
{
  static_assert(cml::are_convertible<value_type_trait_of_t<Sub>,
    value_type_trait_of_t<ASub>, value_type_trait_of_t<RSub>>::value,
    "incompatible scalar types");

  typedef value_type_trait_of_t<Sub>			value_type;

  m.identity();

  vector<value_type, compiled<3>> x, y, z;
  orthonormal_basis(align, reference, x, y, z, normalize, order);
  matrix_set_basis_vectors(m, x, y, z);
}

template<class Sub, class PSub, class TSub, class RSub> void
matrix_rotation_aim_at(
  writable_matrix<Sub>& m,
  const readable_vector<PSub>& pos, const readable_vector<TSub>& target,
  const readable_vector<RSub>& reference,
  axis_order order
  )
{
  matrix_rotation_align(m, target - pos, reference, true, order);
}


/* Conversion: */

template<class Sub, class ASub, class E, class Tol> inline void
matrix_to_axis_angle(
  const readable_matrix<Sub>& m, writable_vector<ASub>& axis,
  E& angle, Tol tolerance
  )
{
  static_assert(cml::are_convertible<
    value_type_trait_of_t<Sub>, value_type_trait_of_t<ASub>, E, Tol>::value,
    "incompatible scalar types");

  typedef value_type_trait_of_t<Sub>			value_type;
  typedef scalar_traits<value_type>			value_traits;
  typedef value_type_trait_of_t<ASub>			asub_type;

  cml::check_linear_3D(m);
  cml::detail::check_or_resize(axis, int_c<3>());

  /* Assign the axis first: */
  axis.set(
    m.basis_element(1,2) - m.basis_element(2,1),
    m.basis_element(2,0) - m.basis_element(0,2),
    m.basis_element(0,1) - m.basis_element(1,0)
    );

  /* Compute the angle: */
  auto l = axis.length();
  auto tmo = trace_3x3(m) - value_type(1);

  /* Normalize and compute the angle directly if possible: */
  if(l > tolerance) {
    axis /= l;
    angle = E(value_traits::atan2(l, tmo));
    /* Note: l = 2*sin(theta), tmo = 2*cos(theta) */
  }

  /* Near-zero axis: */
  else if(tmo > value_type(0)) {
    axis.zero();
    angle = E(0);
  }

  /* Reflection: */
  else {
    auto largest_diagonal_element = cml::index_of_max(
      m.basis_element(0,0), m.basis_element(1,1), m.basis_element(2,2));

    int i, j, k;
    cyclic_permutation(largest_diagonal_element, i, j, k);

    axis[i] = asub_type(value_traits::sqrt(
	m.basis_element(i,i) - m.basis_element(j,j) -
	m.basis_element(k,k) + value_type(1)) / value_type(2));

    auto s = value_type(1) / (value_type(2) * axis[i]);
    axis[j] = asub_type(m.basis_element(i,j) * s);
    axis[k] = asub_type(m.basis_element(i,k) * s);

    angle = constants<E>::pi();
  }

  /* Done. */
}


namespace detail {

/** Helper for the matrix_to_axis_angle() overloads. */
template<class VectorT, class Sub, class Tol> 
inline std::tuple<VectorT, value_type_trait_of_t<Sub>>
matrix_to_axis_angle(const readable_matrix<Sub>& m, Tol tolerance)
{
  static_assert(cml::are_convertible<
    value_type_trait_of_t<Sub>, value_type_trait_of_t<VectorT>, Tol>::value,
    "incompatible scalar types");

  VectorT axis;
  value_type_trait_of_t<Sub> angle;
  cml::matrix_to_axis_angle(m, axis, angle, tolerance);
  return std::make_tuple(std::move(axis), std::move(angle));
}

} // namespace detail

template<class Sub, class Tol>
inline std::tuple<
 vector<value_type_trait_of_t<Sub>, compiled<3>>, value_type_trait_of_t<Sub>
 >
matrix_to_axis_angle(const readable_matrix<Sub>& m, Tol tolerance)
{
  typedef vector<value_type_trait_of_t<Sub>, compiled<3>> vector_type;
  return detail::matrix_to_axis_angle<vector_type, Sub, Tol>(m, tolerance);
}

template<class Sub, class E0, class E1, class E2, class Tol>
inline void
matrix_to_euler(
  const readable_matrix<Sub>& m,
  E0& angle_0, E1& angle_1, E2& angle_2, euler_order order,
  Tol tolerance,
  enable_if_matrix_t<Sub>*
  )
{
  static_assert(cml::are_convertible<
    value_type_trait_of_t<Sub>, E0, E1, E2, Tol>::value,
    "incompatible scalar types");

  typedef value_type_trait_of_t<Sub>			value_type;
  typedef scalar_traits<value_type>			value_traits;

  cml::check_linear_3D(m);

  /* Unpack the order first: */
  int i, j, k;
  bool odd, repeat;
  cml::unpack_euler_order(order, i, j, k, odd, repeat);

  /* Detect repeated indices: */
  if (repeat) {
    auto s1 = cml::length(m.basis_element(j,i), m.basis_element(k,i));
    auto c1 = m.basis_element(i,i);

    angle_1 = E1(value_traits::atan2(s1, c1));
    if (s1 > tolerance) {
      angle_0 = E0(value_traits::atan2(
	m.basis_element(j,i), m.basis_element(k,i)));
      angle_2 = E0(value_traits::atan2(
	m.basis_element(i,j), -m.basis_element(i,k)));
    } else {
      angle_0 = E0(0);
      angle_2 = E2(cml::sign(c1) *
	value_traits::atan2(-m.basis_element(k,j),m.basis_element(j,j)));
    }
  } else {
    auto s1 = -m.basis_element(i,k);
    auto c1 = cml::length(m.basis_element(i,i), m.basis_element(i,j));

    angle_1 = E1(value_traits::atan2(s1, c1));
    if (c1 > tolerance) {
      angle_0 = E0(value_traits::atan2(
	m.basis_element(j,k), m.basis_element(k,k)));
      angle_2 = E2(value_traits::atan2(
	m.basis_element(i,j), m.basis_element(i,i)));
    } else {
      angle_0 = E0(0);
      angle_2 = - E2(cml::sign(s1) *
	value_traits::atan2(-m.basis_element(k,j), m.basis_element(j,j)));
    }
  }

  if(odd) {
    angle_0 = -angle_0;
    angle_1 = -angle_1;
    angle_2 = -angle_2;
  }

  /* Done. */
}


namespace detail {

/** Helper for the matrix_to_euler() overloads. */
template<class VectorT, class Sub, class Tol> inline VectorT
matrix_to_euler(
  const readable_matrix<Sub>& m, euler_order order, Tol tolerance
  )
{
  static_assert(cml::are_convertible<
    value_type_trait_of_t<Sub>, value_type_trait_of_t<VectorT>, Tol>::value,
    "incompatible scalar types");

  VectorT v; cml::detail::check_or_resize(v, int_c<3>());
  cml::matrix_to_euler(m, v[0], v[1], v[2], order, tolerance);
  return std::move(v);
}

} // namespace detail

template<class Sub, class Tol>
inline vector<value_type_trait_of_t<Sub>, compiled<3>>
matrix_to_euler(
  const readable_matrix<Sub>& m, euler_order order, Tol tolerance,
  enable_if_matrix_t<Sub>*
  )
{
  typedef vector<value_type_trait_of_t<Sub>, compiled<3>> vector_type;
  return detail::matrix_to_euler<vector_type, Sub, Tol>(m, order, tolerance);
}

template<class VectorT, class Sub, class Tol>
inline VectorT
matrix_to_euler(
  const readable_matrix<Sub>& m, euler_order order, Tol tolerance,
  enable_if_vector_t<VectorT>*, enable_if_matrix_t<Sub>*
  )
{
  return detail::matrix_to_euler<VectorT, Sub, Tol>(m, order, tolerance);
}

} // namespace cml


#if 0
// XXX INCOMPLETE XXX

//////////////////////////////////////////////////////////////////////////////
// 3D rotation to align with a vector, multiple vectors, or the view plane
//////////////////////////////////////////////////////////////////////////////

/** See vector_ortho.h for details */
template < typename E, class A, class B, class L, class VecT > void
matrix_rotation_align(matrix<E,A,B,L>& m, const VecT& align,
    bool normalize = true, axis_order order = axis_order_zyx)
{
    typedef vector< E,fixed<3> > vector_type;

    identity_transform(m);
    
    vector_type x, y, z;

    orthonormal_basis(align, x, y, z, normalize, order);
    matrix_set_basis_vectors(m, x, y, z);
}

/** See vector_ortho.h for details */
template < typename E,class A,class B,class L,class VecT_1,class VecT_2 > void
matrix_rotation_align_axial(matrix<E,A,B,L>& m, const VecT_1& align,
    const VecT_2& axis, bool normalize = true,
    axis_order order = axis_order_zyx)
{
    typedef vector< E,fixed<3> > vector_type;

    identity_transform(m);
    
    vector_type x, y, z;

    orthonormal_basis_axial(align, axis, x, y, z, normalize, order);
    matrix_set_basis_vectors(m, x, y, z);
}

/** See vector_ortho.h for details */
template < typename E, class A, class B, class L, class MatT > void
matrix_rotation_align_viewplane(
    matrix<E,A,B,L>& m,
    const MatT& view_matrix,
    Handedness handedness,
    axis_order order = axis_order_zyx)
{
    typedef vector< E, fixed<3> > vector_type;

    identity_transform(m);
    
    vector_type x, y, z;

    orthonormal_basis_viewplane(view_matrix, x, y, z, handedness, order);
    matrix_set_basis_vectors(m, x, y, z);
}

/** See vector_ortho.h for details */
template < typename E, class A, class B, class L, class MatT > void
matrix_rotation_align_viewplane_LH(
    matrix<E,A,B,L>& m,
    const MatT& view_matrix,
    axis_order order = axis_order_zyx)
{
    matrix_rotation_align_viewplane(
        m,view_matrix,left_handed,order);
}

/** See vector_ortho.h for details */
template < typename E, class A, class B, class L, class MatT > void
matrix_rotation_align_viewplane_RH(
    matrix<E,A,B,L>& m,
    const MatT& view_matrix,
    axis_order order = axis_order_zyx)
{
    matrix_rotation_align_viewplane(
        m,view_matrix,right_handed,order);
}

//////////////////////////////////////////////////////////////////////////////
// 3D rotation to aim at a target
//////////////////////////////////////////////////////////////////////////////

/** See vector_ortho.h for details */
template < typename E, class A, class B, class L,
    class VecT_1, class VecT_2 > void
matrix_rotation_aim_at(
    matrix<E,A,B,L>& m,
    const VecT_1& pos,
    const VecT_2& target,
    axis_order order = axis_order_zyx)
{
    matrix_rotation_align(m, target - pos, true, order);
}

/** See vector_ortho.h for details */
template < typename E, class A, class B, class L,
    class VecT_1, class VecT_2, class VecT_3 > void
matrix_rotation_aim_at_axial(
    matrix<E,A,B,L>& m,
    const VecT_1& pos,
    const VecT_2& target,
    const VecT_3& axis,
    axis_order order = axis_order_zyx)
{
    matrix_rotation_align_axial(m, target - pos, axis, true, order);
}

//////////////////////////////////////////////////////////////////////////////
// 2D rotation to align with a vector
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// 3D relative rotation about world axes
//////////////////////////////////////////////////////////////////////////////

/** Rotate a rotation matrix about the given world axis */
template < typename E, class A, class B, class L > void
matrix_rotate_about_world_axis(matrix<E,A,B,L>& m, size_t axis, E angle)
{
    typedef matrix<E,A,B,L> matrix_type;
    typedef typename matrix_type::value_type value_type;

    /* Checking */
    detail::CheckMatLinear3D(m);
    detail::CheckIndex3(axis);

    size_t i, j, k;
    cyclic_permutation(axis, i, j, k);

    value_type s = value_type(std::sin(angle));
    value_type c = value_type(std::cos(angle));

    value_type ij = c * m.basis_element(i,j) - s * m.basis_element(i,k);
    value_type jj = c * m.basis_element(j,j) - s * m.basis_element(j,k);
    value_type kj = c * m.basis_element(k,j) - s * m.basis_element(k,k);
    
    m.set_basis_element(i,k, s*m.basis_element(i,j) + c*m.basis_element(i,k));
    m.set_basis_element(j,k, s*m.basis_element(j,j) + c*m.basis_element(j,k));
    m.set_basis_element(k,k, s*m.basis_element(k,j) + c*m.basis_element(k,k));
    
    m.set_basis_element(i,j,ij);
    m.set_basis_element(j,j,jj);
    m.set_basis_element(k,j,kj);
}

/** Rotate a rotation matrix about the world x axis */
template < typename E, class A, class B, class L > void
matrix_rotate_about_world_x(matrix<E,A,B,L>& m, E angle) {
    matrix_rotate_about_world_axis(m,0,angle);
}

/** Rotate a rotation matrix about the world y axis */
template < typename E, class A, class B, class L > void
matrix_rotate_about_world_y(matrix<E,A,B,L>& m, E angle) {
    matrix_rotate_about_world_axis(m,1,angle);
}

/** Rotate a rotation matrix about the world z axis */
template < typename E, class A, class B, class L > void
matrix_rotate_about_world_z(matrix<E,A,B,L>& m, E angle) {
    matrix_rotate_about_world_axis(m,2,angle);
}

//////////////////////////////////////////////////////////////////////////////
// 3D relative rotation about local axes
//////////////////////////////////////////////////////////////////////////////

/** Rotate a rotation matrix about the given local axis */
template < typename E, class A, class B, class L > void
matrix_rotate_about_local_axis(matrix<E,A,B,L>& m, size_t axis, E angle)
{
    typedef matrix<E,A,B,L> matrix_type;
    typedef typename matrix_type::value_type value_type;

    /* Checking */
    detail::CheckMatLinear3D(m);
    detail::CheckIndex3(axis);

    size_t i, j, k;
    cyclic_permutation(axis, i, j, k);

    value_type s = value_type(std::sin(angle));
    value_type c = value_type(std::cos(angle));

    value_type j0 = c * m.basis_element(j,0) + s * m.basis_element(k,0);
    value_type j1 = c * m.basis_element(j,1) + s * m.basis_element(k,1);
    value_type j2 = c * m.basis_element(j,2) + s * m.basis_element(k,2);

    m.set_basis_element(k,0, c*m.basis_element(k,0) - s*m.basis_element(j,0));
    m.set_basis_element(k,1, c*m.basis_element(k,1) - s*m.basis_element(j,1));
    m.set_basis_element(k,2, c*m.basis_element(k,2) - s*m.basis_element(j,2));

    m.set_basis_element(j,0,j0);
    m.set_basis_element(j,1,j1);
    m.set_basis_element(j,2,j2);
}

/** Rotate a rotation matrix about its local x axis */
template < typename E, class A, class B, class L > void
matrix_rotate_about_local_x(matrix<E,A,B,L>& m, E angle) {
    matrix_rotate_about_local_axis(m,0,angle);
}

/** Rotate a rotation matrix about its local y axis */
template < typename E, class A, class B, class L > void
matrix_rotate_about_local_y(matrix<E,A,B,L>& m, E angle) {
    matrix_rotate_about_local_axis(m,1,angle);
}

/** Rotate a rotation matrix about its local z axis */
template < typename E, class A, class B, class L > void
matrix_rotate_about_local_z(matrix<E,A,B,L>& m, E angle) {
    matrix_rotate_about_local_axis(m,2,angle);
}

//////////////////////////////////////////////////////////////////////////////
// 2D relative rotation
//////////////////////////////////////////////////////////////////////////////

template < typename E, class A, class B, class L > void
matrix_rotate_2D(matrix<E,A,B,L>& m, E angle)
{
    typedef matrix<E,A,B,L> matrix_type;
    typedef typename matrix_type::value_type value_type;

    /* Checking */
    detail::CheckMatLinear2D(m);

    value_type s = value_type(std::sin(angle));
    value_type c = value_type(std::cos(angle));

    value_type m00 = c * m.basis_element(0,0) - s * m.basis_element(0,1);
    value_type m10 = c * m.basis_element(1,0) - s * m.basis_element(1,1);

    m.set_basis_element(0,1, s*m.basis_element(0,0) + c*m.basis_element(0,1));
    m.set_basis_element(1,1, s*m.basis_element(1,0) + c*m.basis_element(1,1));

    m.set_basis_element(0,0,m00);
    m.set_basis_element(1,0,m10);
}

//////////////////////////////////////////////////////////////////////////////
// Rotation from vector to vector
//////////////////////////////////////////////////////////////////////////////

/** Build a rotation matrix to rotate from one vector to another
 *
 * Note: The quaternion algorithm is more stable than the matrix algorithm, so
 * we simply pass off to the quaternion function here.
 */
template < class E,class A,class B,class L,class VecT_1,class VecT_2 > void
matrix_rotation_vec_to_vec(
    matrix<E,A,B,L>& m,
    const VecT_1& v1,
    const VecT_2& v2,
    bool unit_length_vectors = false)
{
    typedef quaternion< E,fixed<>,vector_first,positive_cross >
        quaternion_type;
    
    quaternion_type q;
    quaternion_rotation_vec_to_vec(q,v1,v2,unit_length_vectors);
    matrix_rotation_quaternion(m,q);
}

//////////////////////////////////////////////////////////////////////////////
// Scale the angle of a rotation matrix
//////////////////////////////////////////////////////////////////////////////

/** Scale the angle of a 3D rotation matrix */
template < typename E, class A, class B, class L > void
matrix_scale_rotation_angle(matrix<E,A,B,L>& m, E t,
    E tolerance = epsilon<E>::placeholder())
{
    typedef vector< E,fixed<3> > vector_type;
    typedef typename vector_type::value_type value_type;
    
    vector_type axis;
    value_type angle;
    matrix_to_axis_angle(m, axis, angle, tolerance);
    matrix_rotation_axis_angle(m, axis, angle * t);
}

/** Scale the angle of a 2D rotation matrix */
template < typename E, class A, class B, class L > void
matrix_scale_rotation_angle_2D(
    matrix<E,A,B,L>& m, E t, E tolerance = epsilon<E>::placeholder())
{
    typedef vector< E,fixed<2> > vector_type;
    typedef typename vector_type::value_type value_type;

    value_type angle = matrix_to_rotation_2D(m);
    matrix_rotation_2D(m, angle * t);
}

//////////////////////////////////////////////////////////////////////////////
// Support functions for uniform handling of row- and column-basis matrices
//////////////////////////////////////////////////////////////////////////////

/* Note: The matrix rotation slerp, difference and concatenation functions do
 * not use et::MatrixPromote<M1,M2>::temporary_type as the return type, even
 * though that is the return type of the underlying matrix multiplication.
 * This is because the sizes of these matrices are known at compile time (3x3
 * and 2x2), and using fixed<> obviates the need for resizing of intermediate
 * temporaries.
 *
 * Also, no size- or type-checking is done on the arguments to these
 * functions, as any such errors will be caught by the matrix multiplication
 * and assignment to the 3x3 temporary.
 */

/** A fixed-size temporary 3x3 matrix */
#define MAT_TEMP_3X3 matrix<         \
    typename et::ScalarPromote<      \
        typename MatT_1::value_type, \
        typename MatT_2::value_type  \
    >::type,                         \
    fixed<3,3>,                      \
    typename MatT_1::basis_orient,   \
    row_major                        \
>

/** A fixed-size temporary 2x2 matrix */
#define MAT_TEMP_2X2 matrix<         \
    typename et::ScalarPromote<      \
        typename MatT_1::value_type, \
        typename MatT_2::value_type  \
    >::type,                         \
    fixed<2,2>,                      \
    typename MatT_1::basis_orient,   \
    row_major                        \
>

namespace detail {

/** Concatenate two 3D row-basis rotation matrices in the order m1->m2 */
template < class MatT_1, class MatT_2 > MAT_TEMP_3X3
matrix_concat_rotations(const MatT_1& m1, const MatT_2& m2, row_basis) {
    return m1*m2;
}

/** Concatenate two 3D col-basis rotation matrices in the order m1->m2 */
template < class MatT_1, class MatT_2 > MAT_TEMP_3X3
matrix_concat_rotations(const MatT_1& m1, const MatT_2& m2, col_basis) {
    return m2*m1;
}

/** Concatenate two 3D rotation matrices in the order m1->m2 */
template < class MatT_1, class MatT_2 > MAT_TEMP_3X3
matrix_concat_rotations(const MatT_1& m1, const MatT_2& m2) {
    return matrix_concat_rotations(m1,m2,typename MatT_1::basis_orient());
}

/** Concatenate two 2D row-basis rotation matrices in the order m1->m2 */
template < class MatT_1, class MatT_2 > MAT_TEMP_2X2
matrix_concat_rotations_2D(const MatT_1& m1, const MatT_2& m2, row_basis) {
    return m1*m2;
}

/** Concatenate two 2D col-basis rotation matrices in the order m1->m2 */
template < class MatT_1, class MatT_2 > MAT_TEMP_2X2
matrix_concat_rotations_2D(const MatT_1& m1, const MatT_2& m2, col_basis) {
    return m2*m1;
}

/** Concatenate two 2D rotation matrices in the order m1->m2 */
template < class MatT_1, class MatT_2 > MAT_TEMP_2X2
matrix_concat_rotations_2D(const MatT_1& m1, const MatT_2& m2) {
    return matrix_concat_rotations_2D(m1,m2,typename MatT_1::basis_orient());
}

} // namespace detail

//////////////////////////////////////////////////////////////////////////////
// Matrix rotation difference
//////////////////////////////////////////////////////////////////////////////

/** Return the rotational 'difference' between two 3D rotation matrices */
template < class MatT_1, class MatT_2 > MAT_TEMP_3X3
matrix_rotation_difference(const MatT_1& m1, const MatT_2& m2) {
    return detail::matrix_concat_rotations(transpose(m1),m2);
}

/** Return the rotational 'difference' between two 2D rotation matrices */
template < class MatT_1, class MatT_2 > MAT_TEMP_2X2
matrix_rotation_difference_2D(const MatT_1& m1, const MatT_2& m2) {
    return detail::matrix_concat_rotations_2D(transpose(m1),m2);
}

//////////////////////////////////////////////////////////////////////////////
// Spherical linear interpolation of rotation matrices
//////////////////////////////////////////////////////////////////////////////

/* @todo: It might be as fast or faster to simply convert the matrices to
 * quaternions, interpolate, and convert back.
 *
 * @todo: The behavior of matrix slerp is currently a little different than
 * for quaternions: in the matrix function, when the two matrices are close
 * to identical the first is returned, while in the quaternion function the
 * quaternions are nlerp()'d in this case.
 *
 * I still need to do the equivalent of nlerp() for matrices, in which case
 * these functions could be revised to pass off to nlerp() when the matrices
 * are nearly aligned.
*/

/** Spherical linear interpolation of two 3D rotation matrices */
template < class MatT_1, class MatT_2, typename E > MAT_TEMP_3X3
matrix_slerp(const MatT_1& m1, const MatT_2& m2, E t,
    E tolerance = epsilon<E>::placeholder())
{
    typedef MAT_TEMP_3X3 temporary_type;

    temporary_type m = matrix_rotation_difference(m1,m2);
    matrix_scale_rotation_angle(m,t,tolerance);
    return detail::matrix_concat_rotations(m1,m);
}

/** Spherical linear interpolation of two 2D rotation matrices */
template < class MatT_1, class MatT_2, typename E > MAT_TEMP_2X2
matrix_slerp_2D(const MatT_1& m1, const MatT_2& m2, E t,
    E tolerance = epsilon<E>::placeholder())
{
    typedef MAT_TEMP_2X2 temporary_type;

    temporary_type m = matrix_rotation_difference_2D(m1,m2);
    matrix_scale_rotation_angle_2D(m,t,tolerance);
    return detail::matrix_concat_rotations_2D(m1,m);
}

#undef MAT_TEMP_3X3
#undef MAT_TEMP_2X2

//////////////////////////////////////////////////////////////////////////////
// Conversions
//////////////////////////////////////////////////////////////////////////////

/** Convert a 2D rotation matrix to a rotation angle */
template < class MatT > typename MatT::value_type
matrix_to_rotation_2D(const MatT& m)
{
    /* Checking */
    detail::CheckMatLinear2D(m);
    
    return std::atan2(m.basis_element(0,1),m.basis_element(0,0));
}

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
