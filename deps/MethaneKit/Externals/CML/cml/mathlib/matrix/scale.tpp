/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATHLIB_MATRIX_SCALE_TPP
#error "mathlib/matrix/scale.tpp not included correctly"
#endif

#include <cml/common/mpl/are_convertible.h>
#include <cml/vector/size_checking.h>
#include <cml/mathlib/matrix/size_checking.h>

namespace cml {

/* 2D scale: */

template<class Sub, class E0, class E1> inline void
matrix_scale_2D(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1
  )
{
  static_assert(
    cml::are_convertible<value_type_trait_of_t<Sub>, E0, E1>::value,
    "incompatible scalar types");

  cml::check_affine_2D(m);
  m.identity();
  m.set_basis_element(0,0, e0);
  m.set_basis_element(1,1, e1);
}

template<class Sub1, class Sub2> inline void
matrix_scale_2D(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& v
  )
{
  cml::check_size(v, int_c<2>());
  matrix_scale_2D(m, v[0], v[1]);
}

template<class Sub, class E0> inline void
matrix_uniform_scale_2D(
  writable_matrix<Sub>& m, const E0& e0
  )
{
  matrix_scale_2D(m, e0, e0);
}

template<class Sub, class E0, class E1> inline void
matrix_inverse_scale_2D(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1
  )
{
  matrix_scale_2D(m, E0(1)/e0, E1(1)/e1);
}

template<class Sub1, class Sub2> inline void
matrix_inverse_scale_2D(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& v
  )
{
  cml::check_size(v, int_c<2>());
  matrix_inverse_scale_2D(m, v[0], v[1]);
}


/* 3D scaling: */

template<class Sub, class E0, class E1, class E2> inline void
matrix_scale(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1, const E2& e2
  )
{
  static_assert(
    cml::are_convertible<value_type_trait_of_t<Sub>, E0, E1, E2>::value,
    "incompatible scalar types");

  cml::check_affine_3D(m);
  m.identity();
  m.set_basis_element(0,0, e0);
  m.set_basis_element(1,1, e1);
  m.set_basis_element(2,2, e2);
}

template<class Sub1, class Sub2> inline void
matrix_scale(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& v
  )
{
  cml::check_size(v, int_c<3>());
  matrix_scale(m, v[0], v[1], v[2]);
}

template<class Sub, class E0> inline void
matrix_uniform_scale(
  writable_matrix<Sub>& m, const E0& e0
  )
{
  matrix_scale(m, e0, e0, e0);
}

template<class Sub, class E0, class E1, class E2> inline void
matrix_inverse_scale(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1, const E2& e2
  )
{
  matrix_scale(m, E0(1)/e0, E1(1)/e1, E2(1)/e2);
}

template<class Sub1, class Sub2> inline void
matrix_inverse_scale(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& v
  )
{
  cml::check_size(v, int_c<3>());
  matrix_inverse_scale(m, v[0], v[1], v[2]);
}

} // namespace cml


#if 0
// XXX INCOMPLETE XXX

//////////////////////////////////////////////////////////////////////////////
// 3D scale along axis
//////////////////////////////////////////////////////////////////////////////

/** Build a matrix representing a 3D scale along an arbitrary axis */
template < typename E, class A, class B, class L, class VecT > void
matrix_scale_along_axis(matrix<E,A,B,L>&m, const VecT& axis, E scale)
{
    typedef matrix<E,A,B,L> matrix_type;
    typedef typename matrix_type::value_type value_type;

    /* Checking */
    detail::CheckVec3(axis);

    matrix<E,fixed<3,3>,B,L> outer_p = outer(axis,axis)*(scale-value_type(1));
    outer_p(0,0) += value_type(1);
    outer_p(1,1) += value_type(1);
    outer_p(2,2) += value_type(1);

    matrix_linear_transform(m, outer_p);
}

//////////////////////////////////////////////////////////////////////////////
// 2D scale along axis
//////////////////////////////////////////////////////////////////////////////

/** Build a matrix representing a 2D scale along an arbitrary axis */
template < typename E, class A, class B, class L, class VecT >
void matrix_scale_along_axis_2D(matrix<E,A,B,L>&  m, const VecT& axis,
    E scale)
{
    typedef matrix<E,A,B,L> matrix_type;
    typedef typename matrix_type::value_type value_type;

    /* Checking */
    detail::CheckVec2(axis);

    matrix<E,fixed<2,2>,B,L> outer_p = outer(axis,axis)*(scale-value_type(1));
    outer_p(0,0) += value_type(1);
    outer_p(1,1) += value_type(1);

    matrix_linear_transform_2D(m, outer_p);
}

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
