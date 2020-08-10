/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATHLIB_MATRIX_TRANSLATION_TPP
#error "mathlib/matrix/translation.tpp not included correctly"
#endif

#include <cml/common/mpl/are_convertible.h>
#include <cml/vector/size_checking.h>
#include <cml/mathlib/matrix/size_checking.h>

namespace cml {
namespace detail {

/* Base case, set a single basis element (I,J): */
template<int I, int J, class Sub, class E> inline void
matrix_set_basis(writable_matrix<Sub>& m, const E& e)
{
  m.set_basis_element(I, J, e);
}

/* Recursive case: set basis element (I,J) to e, then recursively set
 * (I,J+1):
 */
template<int I, int J, class Sub, class E, class... Es> inline void
matrix_set_basis(writable_matrix<Sub>& m, const E& e, const Es&... es)
{
  m.set_basis_element(I, J, e);
  matrix_set_basis<I,J+1>(m, es...);
}

/* Entry case: set basis vector I from the items in Es: */
template<int I, class Sub, class... Es> inline void
matrix_set_basis(writable_matrix<Sub>& m, const Es&... es)
{
  static_assert(
    cml::are_convertible<value_type_trait_of_t<Sub>, Es...>::value,
    "incompatible scalar types");

  /* Recursively set basis vector I, starting at element 0: */
  matrix_set_basis<I,0>(m, es...);
}

} // namespace detail


/* 2D translation: */

template<class Sub, class E0, class E1> inline void
matrix_set_translation_2D(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1
  )
{
  cml::check_affine_2D(m);
  detail::matrix_set_basis<2>(m, e0, e1);
}

template<class Sub1, class Sub2> inline void
matrix_set_translation_2D(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& v
  )
{
  cml::check_size(v, int_c<2>());
  cml::check_affine_2D(m);
  detail::matrix_set_basis<2>(m, v[0], v[1]);
}


template<class Sub, class E0, class E1> inline void
matrix_get_translation_2D(
  const readable_matrix<Sub>& m, E0& e0, E1& e1
  )
{
  static_assert(
    cml::are_convertible<value_type_trait_of_t<Sub>, E0, E1>::value,
    "incompatible scalar types");

  cml::check_affine_2D(m);
  e0 = E0(m.basis_element(2,0));
  e1 = E1(m.basis_element(2,1));
}

template<class Sub> inline auto
matrix_get_translation_2D(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,2>
{
  cml::check_affine_2D(m);
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
  return n_basis_vector_of_t<Sub,2>(
    m.basis_element(2,0), m.basis_element(2,1));
#else
  return { m.basis_element(2,0), m.basis_element(2,1) };
#endif
}


template<class Sub, class E0, class E1> inline void
matrix_translation_2D(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1
  )
{
  m.identity();
  matrix_set_translation_2D(m, e0, e1);
}

template<class Sub1, class Sub2> inline void
matrix_translation_2D(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& v
  )
{
  m.identity();
  matrix_set_translation_2D(m, v);
}


/* 3D translation: */

template<class Sub, class E0, class E1, class E2> inline void
matrix_set_translation(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1, const E2& e2
  )
{
  cml::check_affine_3D(m);
  detail::matrix_set_basis<3>(m, e0, e1, e2);
}

template<class Sub, class E0, class E1> inline void
matrix_set_translation(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1
  )
{
  typedef value_type_trait_of_t<Sub> zero_type;
  cml::check_affine_3D(m);
  detail::matrix_set_basis<3>(m, e0, e1, zero_type(0));
}

template<class Sub1, class Sub2> inline void
matrix_set_translation(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& v
  )
{
  typedef value_type_trait_of_t<Sub1> zero_type;
  cml::check_size_range(v, int_c<2>(), int_c<3>());
  cml::check_affine_3D(m);
  detail::matrix_set_basis<3>(
    m, v[0], v[1], ((v.size() == 3) ? zero_type(v[2]) : zero_type(0)));
}


template<class Sub, class E0, class E1, class E2> inline void
matrix_get_translation(
  const readable_matrix<Sub>& m, E0& e0, E1& e1, E2& e2
  )
{
  static_assert(
    cml::are_convertible<value_type_trait_of_t<Sub>, E0, E1, E2>::value,
    "incompatible scalar types");

  cml::check_affine_3D(m);
  e0 = E0(m.basis_element(3,0));
  e1 = E1(m.basis_element(3,1));
  e2 = E2(m.basis_element(3,2));
}

template<class Sub> inline auto
matrix_get_translation(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,3>
{
  cml::check_affine_3D(m);
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
  return n_basis_vector_of_t<Sub,3>(
    m.basis_element(3,0), m.basis_element(3,1), m.basis_element(3,2));
#else
  return { m.basis_element(3,0), m.basis_element(3,1), m.basis_element(3,2) };
#endif
}


template<class Sub, class E0, class E1, class E2> inline void
matrix_translation(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1, const E2& e2
  )
{
  m.identity();
  matrix_set_translation(m, e0, e1, e2);
}

template<class Sub1, class Sub2> inline void
matrix_translation(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& v
  )
{
  m.identity();
  matrix_set_translation(m, v);
}

template<class Sub, class E0, class E1> inline void
matrix_translation(
  writable_matrix<Sub>& m, const E0& e0, const E1& e1
  )
{
  m.identity();
  matrix_set_translation(m, e0, e1);
}

} // namespace cml



#if 0
// XXX INCOMPLETE XXX

//////////////////////////////////////////////////////////////////////////////
// Function for getting the translation of a 3D view matrix
//////////////////////////////////////////////////////////////////////////////

/** Get the translation of a 3D affine transform */
template < class MatT > vector< typename MatT::value_type, fixed<3> >
matrix_get_view_translation(const MatT& m)
{
    typedef typename MatT::value_type value_type;
    typedef vector< value_type, fixed<3> > vector_type;
    
    vector_type x, y, z;
    matrix_get_basis_vectors(m,x,y,z);
    vector_type p = matrix_get_translation(m);
    return vector_type(-dot(p,x),-dot(p,y),-dot(p,z));
}

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
