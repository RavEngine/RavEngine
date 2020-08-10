/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATHLIB_VECTOR_ORTHONORMAL_TPP
#error "mathlib/vector/orthonormal.tpp not included correctly"
#endif

#include <cml/common/mpl/are_convertible.h>
#include <cml/vector/cross.h>
#include <cml/vector/fixed_compiled.h>
#include <cml/mathlib/vector/misc.h>

namespace cml {

template<class Sub, class XSub, class YSub> void
orthonormal_basis_2D(
  const readable_vector<Sub>& align,
  writable_vector<XSub>& x, writable_vector<YSub>& y,
  bool normalize_align, axis_order2D order)
{
  static_assert(cml::are_convertible<value_type_trait_of_t<Sub>,
    value_type_trait_of_t<XSub>, value_type_trait_of_t<YSub>>::value,
    "incompatible scalar types");

  typedef value_type_trait_of_t<Sub>			value_type;
  typedef vector<value_type, compiled<2>>		temporary_type;

  /* Checking handled by perp() and assignment to fixed<2>. */

  int i, j;
  bool odd;
  unpack_axis_order2D(order, i, j, odd);

  temporary_type axes[2];

  axes[i] = align; if(normalize_align) axes[i].normalize();
  axes[j] = cml::perp(axes[i]);

  if(odd) axes[j] = -axes[j];

  x = axes[0];
  y = axes[1];
}

template<class Sub1, class Sub2, class XSub, class YSub, class ZSub> void
orthonormal_basis(
  const readable_vector<Sub1>& align, const readable_vector<Sub2>& reference,
  writable_vector<XSub>& x, writable_vector<YSub>& y, writable_vector<ZSub>& z,
  bool normalize_align, axis_order order
  )
{
  static_assert(cml::are_convertible<value_type_trait_of_t<Sub1>,
    value_type_trait_of_t<Sub2>, value_type_trait_of_t<XSub>,
    value_type_trait_of_t<YSub>, value_type_trait_of_t<ZSub>>::value,
    "incompatible scalar types");

  typedef value_type_trait_of_t<Sub1>			value_type;
  typedef vector<value_type, compiled<3>>		temporary_type;

  int i, j, k;
  bool odd;
  unpack_axis_order(order, i, j, k, odd);

  temporary_type axes[3];
  axes[i] = align; if(normalize_align) axes[i].normalize();
  axes[k] = cml::cross(axes[i], reference).normalize();
  axes[j] = cml::cross(axes[k], axes[i]);
 
  if(odd) axes[k] = - axes[k];

  x = axes[0];
  y = axes[1];
  z = axes[2];
}

} // namespace cml


#if 0
// XXX INCOMPLETE XXX

//////////////////////////////////////////////////////////////////////////////
// Orthonormalization in 3D and 2D
//////////////////////////////////////////////////////////////////////////////


/** Orthonormalize 3 basis vectors in R3.
 *
 * Called with the default values, this function performs a single Gram-
 * Schmidt step to orthonormalize the input vectors. By default, the direction
 * of the 3rd basis vector is unchanged by this operation, but the unaffected
 * axis can be specified via the 'stable_axis' parameter.
 *
 * The arguments 'num_iter' and 's' can be specified to an iterative Gram-
 * Schmidt step. 'num_iter' is the number of iterations applied, and 's' is
 * the fraction applied towards orthonormality each step.
 *
 * In most cases, the default arguments can be ignored, leaving only the three
 * input vectors.
 */
template < typename E, class A > void
orthonormalize(vector<E,A>& v0, vector<E,A>& v1, vector<E,A>& v2,
    size_t stable_axis = 2, size_t num_iter = 0, E s = E(1))
{
    /* Checking */
    detail::CheckVec3(v0);
    detail::CheckVec3(v1);
    detail::CheckVec3(v2);
    detail::CheckIndex3(stable_axis);

    typedef vector< E, fixed<3> > vector_type;
    typedef typename vector_type::value_type value_type;

    /* Iterative Gram-Schmidt; this step is skipped by default. */
    
    for (size_t i = 0; i < num_iter; ++i) {
        value_type dot01 = dot(v0,v1);
        value_type dot12 = dot(v1,v2);
        value_type dot20 = dot(v2,v0);
        value_type inv_dot00 = value_type(1) / dot(v0,v0);
        value_type inv_dot11 = value_type(1) / dot(v1,v1);
        value_type inv_dot22 = value_type(1) / dot(v2,v2);

        vector_type temp0 = v0 - s*dot01*inv_dot11*v1 - s*dot20*inv_dot22*v2;
        vector_type temp1 = v1 - s*dot12*inv_dot22*v2 - s*dot01*inv_dot00*v0;
        vector_type temp2 = v2 - s*dot20*inv_dot00*v0 - s*dot12*inv_dot11*v1;
        
        v0 = temp0;
        v1 = temp1;
        v2 = temp2;
    }

    /* Final Gram-Schmidt step to ensure orthonormality. If no iterations
     * have been requested (num_iter = 0), this is the only step. The step
     * is performed such that the direction of the axis indexed by
     * 'stable_axis' is unchanged.
     */

    size_t i, j, k;
    cyclic_permutation(stable_axis, i, j, k);
    vector_type v[] = { v0, v1, v2 };

    v[i].normalize();
    v[j] = normalize(project_to_hplane(v[j],v[i]));
    v[k] = normalize(project_to_hplane(project_to_hplane(v[k],v[i]),v[j]));
    
    v0 = v[0];
    v1 = v[1];
    v2 = v[2];
}

/** Orthonormalize 2 basis vectors in R2 */
template < typename E, class A > void
orthonormalize(vector<E,A>& v0, vector<E,A>& v1,
    size_t stable_axis = 0, size_t num_iter = 0, E s = E(1))
{
    typedef vector< E, fixed<2> > vector_type;
    typedef typename vector_type::value_type value_type;

    /* Checking */
    detail::CheckVec2(v0);
    detail::CheckVec2(v1);
    detail::CheckIndex2(stable_axis);

    /* Iterative Gram-Schmidt; this step is skipped by default. */
    
    for (size_t i = 0; i < num_iter; ++i) {
        value_type dot01 = dot(v0,v1);

        vector_type temp0 = v0 - (s * dot01 * v1) / dot(v1,v1);
        vector_type temp1 = v1 - (s * dot01 * v0) / dot(v0,v0);
        
        v0 = temp0;
        v1 = temp1;
    }

    /* Final Gram-Schmidt step to ensure orthonormality. If no iterations
     * have been requested (num_iter = 0), this is the only step. The step
     * is performed such that the direction of the axis indexed by
     * 'stable_axis' is unchanged.
     */

    size_t i, j;
    cyclic_permutation(stable_axis, i, j);
    vector_type v[] = { v0, v1 };

    v[i].normalize();
    v[j] = normalize(project_to_hplane(v[j],v[i]));
    
    v0 = v[0];
    v1 = v[1];
}

//////////////////////////////////////////////////////////////////////////////
// Orthonormal basis construction in 3D and 2D
//////////////////////////////////////////////////////////////////////////////

/** This version of orthonormal_basis() constructs in arbitrary basis given a
 * vector with which to align the i'th basis vector. To avoid the failure
 * case, the reference vector is always chosen so as to not be parallel to
 * 'align'. This means the algorithm will always generate a valid basis, which
 * can be useful in some circumstances; however, it should be noted that the
 * basis will likely 'pop' as the alignment vector changes, and so may not be
 * suitable for billboarding or other similar applications.
 */
template < class VecT, typename E, class A >
void orthonormal_basis(
    const VecT& align,
    vector<E,A>& x,
    vector<E,A>& y,
    vector<E,A>& z,
    bool normalize_align = true,
    axis_order order = axis_order_zyx)
{
    /* Checking (won't be necessary with index_of_min_abs() member function */
    detail::CheckVec3(align);

    /* @todo: vector member function index_of_min_abs() would clean this up */
    
    orthonormal_basis(
        align,
        axis_3D(cml::index_of_min_abs(align[0],align[1],align[2])),
        x, y, z, normalize_align, order
    );
}

/** orthonormal_basis_axial() generates a basis in which the j'th basis vector
 * is aligned with 'axis' and the i'th basis vector is maximally aligned (as
 * 'aligned as possible') with 'align'. This can be used for e.g. axial
 * billboarding for, say, trees or beam effects.
 *
 * Note that the implementation simply passes off to the 'reference' version
 * of orthonormal_basis(), with the parameters adjusted so that the alignment
 * is axial.
 *
 * @note With this algorithm the failure case is when 'align' and 'axis'
 * are nearly parallel; if this is likely, it should be checked for and
 * handled externally.
 */
template < class VecT_1, class VecT_2, typename E, class A >
void orthonormal_basis_axial(
    const VecT_1& align,
    const VecT_2& axis,
    vector<E,A>& x,
    vector<E,A>& y,
    vector<E,A>& z,
    bool normalize_align = true,
    axis_order order = axis_order_zyx)
{
    orthonormal_basis(
        axis,
        align,
        x,
        y,
        z,
        normalize_align,
        detail::swap_axis_order(order));
}

/** orthonormal_basis_viewplane() builds a basis aligned with a viewplane, as
 * extracted from the input view matrix. The function takes into account the
 * handedness of the input view matrix and orients the basis accordingly.
 *
 * @note The generated basis will always be valid.
 */
template < class MatT, typename E, class A >
void orthonormal_basis_viewplane(
    const MatT& view_matrix,
    vector<E,A>& x,
    vector<E,A>& y,
    vector<E,A>& z,
    Handedness handedness,
    axis_order order = axis_order_zyx)
{
    typedef MatT matrix_type;
    typedef typename matrix_type::value_type value_type;

    orthonormal_basis(
        -(handedness == left_handed ? value_type(1) : value_type(-1)) *
            matrix_get_transposed_z_basis_vector(view_matrix),
        matrix_get_transposed_y_basis_vector(view_matrix),
        x, y, z, false, order
    );
}

/** Build a viewplane-oriented basis from a left-handedness view matrix. */
template < class MatT, typename E, class A >
void orthonormal_basis_viewplane_LH(
    const MatT& view_matrix,
    vector<E,A>& x,
    vector<E,A>& y,
    vector<E,A>& z,
    axis_order order = axis_order_zyx)
{
    orthonormal_basis_viewplane(
        view_matrix,x,y,z,left_handed,order);
}

/** Build a viewplane-oriented basis from a right-handedness view matrix. */
template < class MatT, typename E, class A >
void orthonormal_basis_viewplane_RH(
    const MatT& view_matrix,
    vector<E,A>& x,
    vector<E,A>& y,
    vector<E,A>& z,
    axis_order order = axis_order_zyx)
{
    orthonormal_basis_viewplane(
        view_matrix,x,y,z,right_handed,order);
}
#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
