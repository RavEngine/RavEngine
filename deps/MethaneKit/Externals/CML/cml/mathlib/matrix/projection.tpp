/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATHLIB_MATRIX_PROJECTION_TPP
#error "mathlib/matrix/projection.tpp not included correctly"
#endif

#include <cml/common/mpl/are_convertible.h>
#include <cml/mathlib/matrix/size_checking.h>

namespace cml {

/* Orthographic projection functions: */

/** Build a matrix representing an orthographic projection, specified by
 *  frustum bounds in l,r,b,t,n,f form, and with the given handedness and z
 *  clipping range
 */

template<class Sub, class E> inline void
matrix_orthographic(
  writable_matrix<Sub>& m,
  E left, E right, E bottom, E top, E n, E f,
  AxisOrientation handedness, ZClip z_clip
  )
{
  static_assert(
    cml::are_convertible<value_type_trait_of_t<Sub>, E>::value,
    "incompatible scalar types");

  cml::check_minimum_size(m, int_c<4>(), int_c<4>());

  /* Initialize: */
  m.identity();
    
  auto inv_width = E(1) / (right - left);
  auto inv_height = E(1) / (top - bottom);
  auto inv_depth = E(1) / (f - n);
  auto s = E(handedness == left_handed ? 1 : -1);

  if (z_clip == z_clip_neg_one) {
    m.set_basis_element(2,2, s * E(2) * inv_depth);
    m.set_basis_element(3,2, -(f + n) * inv_depth);
  } else { // z_clip.z_clip() == 0
    m.set_basis_element(2,2, s * inv_depth);
    m.set_basis_element(3,2, -n * inv_depth);
  }

  m.set_basis_element(0,0, E(2) * inv_width   );
  m.set_basis_element(1,1, E(2) * inv_height  );
  m.set_basis_element(3,0, -(right + left) * inv_width );
  m.set_basis_element(3,1, -(top + bottom) * inv_height);
}

template<class Sub, class E> inline void
matrix_orthographic_LH(
  writable_matrix<Sub>& m,
  E left, E right, E bottom, E top, E n, E f,
  ZClip z_clip
  )
{
  matrix_orthographic(m, left, right, bottom, top, n, f, left_handed, z_clip);
}

template<class Sub, class E> inline void
matrix_orthographic_RH(
  writable_matrix<Sub>& m,
  E left, E right, E bottom, E top, E n, E f,
  ZClip z_clip
  )
{
  matrix_orthographic(m, left, right, bottom, top, n, f, right_handed, z_clip);
}


template<class Sub, class E> inline void
matrix_orthographic(
  writable_matrix<Sub>& m,
  E width, E height, E n, E f,
  AxisOrientation handedness, ZClip z_clip
  )
{
  auto half_width = width / E(2);
  auto half_height = height / E(2);
  matrix_orthographic(m,
    -half_width, half_width, -half_height, half_height,
    n, f, handedness, z_clip);
}

template<class Sub, class E> inline void
matrix_orthographic_LH(
  writable_matrix<Sub>& m,
  E width, E height, E n, E f,
  ZClip z_clip
  )
{
  matrix_orthographic(m, width, height, n, f, left_handed, z_clip);
}

template<class Sub, class E> inline void
matrix_orthographic_RH(
  writable_matrix<Sub>& m,
  E width, E height, E n, E f,
  ZClip z_clip
  )
{
  matrix_orthographic(m, width, height, n, f, right_handed, z_clip);
}



/* Perspective projection functions: */

template<class Sub, class E> inline void
matrix_perspective(
  writable_matrix<Sub>& m,
  E left, E right, E bottom, E top, E n, E f,
  AxisOrientation handedness, ZClip z_clip
  )
{
  static_assert(
    cml::are_convertible<value_type_trait_of_t<Sub>, E>::value,
    "incompatible scalar types");

  cml::check_minimum_size(m, int_c<4>(), int_c<4>());

  /* Initialize: */
  m.identity();

  auto inv_width = E(1) / (right - left);
  auto inv_height = E(1) / (top - bottom);
  auto inv_depth = E(1) / (f - n);
  auto near2 = E(2) * n;
  auto s = E(handedness == left_handed ? 1 : -1);

  if (z_clip == z_clip_neg_one) {
    m.set_basis_element(2,2, s * (f + n) * inv_depth);
    m.set_basis_element(3,2, - E(2) * f * n * inv_depth);
  } else { // z_clip == z_clip_zero
    m.set_basis_element(2,2,  s * f * inv_depth);
    m.set_basis_element(3,2, -s * n * m.basis_element(2,2));
  }

  m.set_basis_element(0,0, near2 * inv_width               );
  m.set_basis_element(1,1, near2 * inv_height              );
  m.set_basis_element(2,0, -s * (right + left) * inv_width );
  m.set_basis_element(2,1, -s * (top + bottom) * inv_height);
  m.set_basis_element(2,3, s                               );
  m.set_basis_element(3,3, 0                               );
}

template<class Sub, class E> inline void
matrix_perspective_LH(
  writable_matrix<Sub>& m,
  E left, E right, E bottom, E top, E n, E f,
  ZClip z_clip
  )
{
  matrix_perspective(m, left, right, bottom, top, n, f, left_handed, z_clip);
}

template<class Sub, class E> inline void
matrix_perspective_RH(
  writable_matrix<Sub>& m,
  E left, E right, E bottom, E top, E n, E f,
  ZClip z_clip
  )
{
  matrix_perspective(m, left, right, bottom, top, n, f, right_handed, z_clip);
}


template<class Sub, class E> inline void
matrix_perspective(
  writable_matrix<Sub>& m,
  E width, E height, E n, E f,
  AxisOrientation handedness, ZClip z_clip
  )
{
  auto half_width = width / E(2);
  auto half_height = height / E(2);
  matrix_perspective(m,
    -half_width, half_width, -half_height, half_height,
    n, f, handedness, z_clip);
}

template<class Sub, class E> inline void
matrix_perspective_LH(
  writable_matrix<Sub>& m,
  E width, E height, E n, E f,
  ZClip z_clip
  )
{
  matrix_perspective(m, width, height, n, f, left_handed, z_clip);
}

template<class Sub, class E> inline void
matrix_perspective_RH(
  writable_matrix<Sub>& m,
  E width, E height, E n, E f,
  ZClip z_clip
  )
{
  matrix_perspective(m, width, height, n, f, right_handed, z_clip);
}


template<class Sub, class E> inline void
matrix_perspective_xfov(
  writable_matrix<Sub>& m,
  E xfov, E aspect, E n, E f,
  AxisOrientation handedness, ZClip z_clip
  )
{
  typedef scalar_traits<E>				E_traits;

  /* Compute the view height from the field of view: */
  auto width = E(2) * n * E_traits::tan(xfov / E(2));
  matrix_perspective(m, width, width / aspect, n, f, handedness, z_clip);
}

template<class Sub, class E> inline void
matrix_perspective_xfov_LH(
  writable_matrix<Sub>& m,
  E xfov, E aspect, E n, E f,
  ZClip z_clip
  )
{
  matrix_perspective_xfov(m, xfov, aspect, n, f, left_handed, z_clip);
}

template<class Sub, class E> inline void
matrix_perspective_xfov_RH(
  writable_matrix<Sub>& m,
  E xfov, E aspect, E n, E f,
  ZClip z_clip
  )
{
  matrix_perspective_xfov(m, xfov, aspect, n, f, right_handed, z_clip);
}


template<class Sub, class E> inline void
matrix_perspective_yfov(
  writable_matrix<Sub>& m,
  E yfov, E aspect, E n, E f,
  AxisOrientation handedness, ZClip z_clip
  )
{
  typedef scalar_traits<E>				E_traits;

  /* Compute the view height from the field of view: */
  auto height = E(2) * n * E_traits::tan(yfov / E(2));
  matrix_perspective(m, height * aspect, height, n, f, handedness, z_clip);
}

template<class Sub, class E> inline void
matrix_perspective_yfov_LH(
  writable_matrix<Sub>& m,
  E yfov, E aspect, E n, E f,
  ZClip z_clip
  )
{
  matrix_perspective_yfov(m, yfov, aspect, n, f, left_handed, z_clip);
}

template<class Sub, class E> inline void
matrix_perspective_yfov_RH(
  writable_matrix<Sub>& m,
  E yfov, E aspect, E n, E f,
  ZClip z_clip
  )
{
  matrix_perspective_yfov(m, yfov, aspect, n, f, right_handed, z_clip);
}

} // namespace cml

#if 0
// XXX INCOMPLETE XXX

/* Build a viewport matrix
 *
 * Note: A viewport matrix is in a sense the opposite of an orthographics
 * projection matrix, and can be build by constructing and inverting the
 * latter.
 *
 * @todo: Need to look into D3D viewport conventions and see if this needs to
 * be adapted accordingly.
 */

template < typename E, class A, class B, class L > void
matrix_viewport(matrix<E,A,B,L>& m, E left, E right, E bottom,
    E top, ZClip z_clip, E n = E(0), E f = E(1))
{
    matrix_orthographic_LH(m, left, right, bottom, top, n, f, z_clip);
    /* @todo: invert(m), when available */
    m = inverse(m);
}

//////////////////////////////////////////////////////////////////////////////
// 3D picking volume
//////////////////////////////////////////////////////////////////////////////

/* Build a pick volume matrix
 *
 * When post-concatenated with a projection matrix, the pick matrix modifies
 * the view volume to create a 'picking volume'. This volume corresponds to
 * a screen rectangle centered at (pick_x, pick_y) and with dimensions
 * pick_widthXpick_height.
 *
 * @todo: Representation of viewport between this function and
 * matrix_viewport() is inconsistent (position and dimensions vs. bounds).
 * Should this be addressed?
 */

template < typename E, class A, class B, class L > void
matrix_pick(
    matrix<E,A,B,L>& m, E pick_x, E pick_y, E pick_width, E pick_height,
    E viewport_x, E viewport_y, E viewport_width, E viewport_height)
{
    typedef matrix<E,A,B,L> matrix_type;
    typedef typename matrix_type::value_type value_type;

    /* Checking */
    detail::CheckMatHomogeneous3D(m);

    identity_transform(m);
    
    value_type inv_width = value_type(1) / pick_width;
    value_type inv_height = value_type(1) / pick_height;
    
    m.set_basis_element(0,0,viewport_width*inv_width);
    m.set_basis_element(1,1,viewport_height*inv_height);
    m.set_basis_element(3,0,
        (viewport_width+value_type(2)*(viewport_x-pick_x))*inv_width);
    m.set_basis_element(3,1,
        (viewport_height+value_type(2)*(viewport_y-pick_y))*inv_height);
}
#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
