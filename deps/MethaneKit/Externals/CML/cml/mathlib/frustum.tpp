/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATHLIB_FRUSTUM_TPP
#error "mathlib/frustum.tpp not included correctly"
#endif

#include <cml/common/mpl/are_convertible.h>
#include <cml/matrix/size_checking.h>
#include <cml/mathlib/matrix/concat.h>

namespace cml {

template<class Sub1, class Sub2, class E> void
extract_frustum_planes(
  const readable_matrix<Sub1>& modelview,
  const readable_matrix<Sub2>& projection,
  E planes[6][4],
  ZClip z_clip,
  bool normalize
  )
{
  extract_frustum_planes(
    matrix_concat(modelview, projection), planes, z_clip, normalize);
}

template<class Sub, class E> void
extract_frustum_planes(
  const readable_matrix<Sub>& m,
  E planes[6][4],
  ZClip z_clip,
  bool normalize
  )
{
  static_assert(
    cml::are_convertible<value_type_trait_of_t<Sub>, E>::value,
    "incompatible scalar types");

  /* Near:   [03+02, 13+12, 23+22, 33+32] : [02, 12, 22, 32] */
  extract_near_frustum_plane(m, planes[4], z_clip);
  /* Note: also checks that m is at least 4x4. */

  /* Left:   [03+00, 13+10, 23+20, 33+30] */

  planes[0][0] = E(m.basis_element(0,3) + m.basis_element(0,0));
  planes[0][1] = E(m.basis_element(1,3) + m.basis_element(1,0));
  planes[0][2] = E(m.basis_element(2,3) + m.basis_element(2,0));  
  planes[0][3] = E(m.basis_element(3,3) + m.basis_element(3,0));

  /* Right:  [03-00, 13-10, 23-20, 33-30] */

  planes[1][0] = E(m.basis_element(0,3) - m.basis_element(0,0));
  planes[1][1] = E(m.basis_element(1,3) - m.basis_element(1,0));
  planes[1][2] = E(m.basis_element(2,3) - m.basis_element(2,0));  
  planes[1][3] = E(m.basis_element(3,3) - m.basis_element(3,0));

  /* Bottom: [03+01, 13+11, 23+21, 33+31] */

  planes[2][0] = E(m.basis_element(0,3) + m.basis_element(0,1));
  planes[2][1] = E(m.basis_element(1,3) + m.basis_element(1,1));
  planes[2][2] = E(m.basis_element(2,3) + m.basis_element(2,1));  
  planes[2][3] = E(m.basis_element(3,3) + m.basis_element(3,1));

  /* Top:    [03-01, 13-11, 23-21, 33-31] */

  planes[3][0] = E(m.basis_element(0,3) - m.basis_element(0,1));
  planes[3][1] = E(m.basis_element(1,3) - m.basis_element(1,1));
  planes[3][2] = E(m.basis_element(2,3) - m.basis_element(2,1));  
  planes[3][3] = E(m.basis_element(3,3) - m.basis_element(3,1));

  /* Far:    [03-02, 13-12, 23-22, 33-32] */

  planes[5][0] = E(m.basis_element(0,3) - m.basis_element(0,2));
  planes[5][1] = E(m.basis_element(1,3) - m.basis_element(1,2));
  planes[5][2] = E(m.basis_element(2,3) - m.basis_element(2,2));  
  planes[5][3] = E(m.basis_element(3,3) - m.basis_element(3,2));

  /* Compute normalized planes: */
  if(normalize) {
    for(int i = 0; i < 6; ++i) {
      auto l = cml::length(planes[i][0], planes[i][1], planes[i][2]);
      planes[i][0] /= l;
      planes[i][1] /= l;
      planes[i][2] /= l;
      planes[i][3] /= l;
    }
  }
}

template<class Sub, class Plane> void
extract_near_frustum_plane(
  const readable_matrix<Sub>& m, Plane& plane, ZClip z_clip
  )
{
  cml::check_minimum_size(m, cml::int_c<4>(), cml::int_c<4>());
  //XXX cml::check_minimum_size(plane, cml::int_c<4>());

  /* Near:   [03+02, 13+12, 23+22, 33+32] : [02, 12, 22, 32] */
  if (z_clip == z_clip_neg_one) {       
    plane[0] = m.basis_element(0,3) + m.basis_element(0,2);
    plane[1] = m.basis_element(1,3) + m.basis_element(1,2);
    plane[2] = m.basis_element(2,3) + m.basis_element(2,2);  
    plane[3] = m.basis_element(3,3) + m.basis_element(3,2);
  } else { // z_clip == z_clip_zero
    plane[0] = m.basis_element(0,2);
    plane[1] = m.basis_element(1,2);
    plane[2] = m.basis_element(2,2);  
    plane[3] = m.basis_element(3,2);
  }
}

} // namespace cml

#if 0
// XXX INCOMPLETE XXX

/* @todo: plane class, and perhaps named arguments instead of an array. */

namespace detail {

/* This is currently only in support of finding the corners of a frustum.
 * The input planes are assumed to have a single unique intersection, so
 * no tolerance is used.
 */

template < typename Real > vector< Real, fixed<3> >
intersect_planes(Real p1[4], Real p2[4], Real p3[4])
{
    typedef vector< Real, fixed<3> > vector_type;
    typedef typename vector_type::value_type value_type;

    vector_type n1(p1[0],p1[1],p1[2]);
    vector_type n2(p2[0],p2[1],p2[2]);
    vector_type n3(p3[0],p3[1],p3[2]);
    
    value_type d1 = -p1[3];
    value_type d2 = -p2[3];
    value_type d3 = -p3[3];
    
    vector_type numer =
        d1*cross(n2,n3) + d2*cross(n3,n1) + d3*cross(n1,n2);
    value_type denom = triple_product(n1,n2,n3);
    return numer/denom;
}

} // namespace detail

/* Get the corners of a frustum defined by 6 planes. The planes are in
 * ax+by+cz+d = 0 form, and are in the order:
 *     left
 *     right
 *     bottom
 *     top
 *     near
 *     far
 *
 * The corners are in CCW order starting in the lower-left, first at the near
 * plane, then at the far plane.
 */

template < typename Real, typename E, class A > void
get_frustum_corners(Real planes[6][4], vector<E,A> corners[8])
{
    // NOTE: Prefixed with 'PLANE_' due to symbol conflict with Windows
    // macros PLANE_LEFT and PLANE_RIGHT.
    enum {
        PLANE_LEFT,
        PLANE_RIGHT,
        PLANE_BOTTOM,
        PLANE_TOP,
        PLANE_NEAR,
        PLANE_FAR
    };

    corners[0] = detail::intersect_planes(
        planes[PLANE_LEFT],
        planes[PLANE_BOTTOM],
        planes[PLANE_NEAR]
    );
    corners[1] = detail::intersect_planes(
        planes[PLANE_RIGHT],
        planes[PLANE_BOTTOM],
        planes[PLANE_NEAR]
    );
    corners[2] = detail::intersect_planes(
        planes[PLANE_RIGHT],
        planes[PLANE_TOP],
        planes[PLANE_NEAR]
    );
    corners[3] = detail::intersect_planes(
        planes[PLANE_LEFT],
        planes[PLANE_TOP],
        planes[PLANE_NEAR]
    );
    corners[4] = detail::intersect_planes(
        planes[PLANE_LEFT],
        planes[PLANE_BOTTOM],
        planes[PLANE_FAR]
    );
    corners[5] = detail::intersect_planes(
        planes[PLANE_RIGHT],
        planes[PLANE_BOTTOM],
        planes[PLANE_FAR]
    );
    corners[6] = detail::intersect_planes(
        planes[PLANE_RIGHT],
        planes[PLANE_TOP],
        planes[PLANE_FAR]
    );
    corners[7] = detail::intersect_planes(
        planes[PLANE_LEFT],
        planes[PLANE_TOP],
        planes[PLANE_FAR]
    );
}

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
