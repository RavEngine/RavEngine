/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATHLIB_VECTOR_TRANSFORM_TPP
#error "mathlib/vector/transform.tpp not included correctly"
#endif

#include <cml/vector/fixed_compiled.h>
#include <cml/matrix/vector_product.h>
#include <cml/mathlib/matrix/size_checking.h>

namespace cml {
namespace detail {

/** Matrix-vector pre-multiplication. */
template<class Sub1, class Sub2> inline auto
transform_vector_4D(
  const readable_matrix<Sub1>& m, const readable_vector<Sub2>& v, col_basis
  ) -> temporary_of_t<Sub2>
{
  return m * v;
}

/** Matrix-vector post-multiplication. */
template<class Sub1, class Sub2> inline auto
transform_vector_4D(
  const readable_matrix<Sub1>& m, const readable_vector<Sub2>& v, row_basis
  ) -> temporary_of_t<Sub2>
{
  return v * m;
}

} // namespace detail


/* 2D transformations: */

template<class Sub1, class Sub2> inline auto
transform_vector_2D(
  const readable_matrix<Sub1>& m, const readable_vector<Sub2>& v
  ) -> temporary_of_t<Sub2>
{
  typedef temporary_of_t<Sub2>				result_type;
  typedef value_type_trait_of_t<result_type>		value_type;
  cml::check_minimum_size(m, int_c<2>(), int_c<2>());
  cml::check_size(v, int_c<2>());
  return result_type(
    value_type(m.basis_element(0,0)*v[0] + m.basis_element(1,0)*v[1]),
    value_type(m.basis_element(0,1)*v[0] + m.basis_element(1,1)*v[1])
    );
}

template<class Sub1, class Sub2> inline auto
transform_point_2D(
  const readable_matrix<Sub1>& m, const readable_vector<Sub2>& v
  ) -> temporary_of_t<Sub2>
{
  typedef temporary_of_t<Sub2>				result_type;
  typedef value_type_trait_of_t<result_type>		value_type;
  cml::check_affine_2D(m);
  cml::check_size(v, int_c<2>());
  return result_type(
    value_type(
      m.basis_element(0,0)*v[0] +
      m.basis_element(1,0)*v[1] +
      m.basis_element(2,0)),

    value_type(
      m.basis_element(0,1)*v[0] +
      m.basis_element(1,1)*v[1] +
      m.basis_element(2,1))
    );
}


/* 3D transformations: */

template<class Sub1, class Sub2> inline auto
transform_vector(
  const readable_matrix<Sub1>& m, const readable_vector<Sub2>& v
  ) -> temporary_of_t<Sub2>
{
  typedef temporary_of_t<Sub2>				result_type;
  typedef value_type_trait_of_t<result_type>		value_type;
  cml::check_minimum_size(m, int_c<3>(), int_c<3>());
  cml::check_size(v, int_c<3>());
  return result_type(
    value_type(
      m.basis_element(0,0)*v[0] +
      m.basis_element(1,0)*v[1] +
      m.basis_element(2,0)*v[2]),

    value_type(
      m.basis_element(0,1)*v[0] +
      m.basis_element(1,1)*v[1] +
      m.basis_element(2,1)*v[2]),

    value_type(
      m.basis_element(0,2)*v[0] +
      m.basis_element(1,2)*v[1] +
      m.basis_element(2,2)*v[2])
    );
}

template<class Sub1, class Sub2> inline auto
transform_point(
  const readable_matrix<Sub1>& m, const readable_vector<Sub2>& v
  ) -> temporary_of_t<Sub2>
{
  typedef temporary_of_t<Sub2>				result_type;
  typedef value_type_trait_of_t<result_type>		value_type;
  cml::check_affine_3D(m);
  cml::check_size(v, int_c<3>());
  return result_type(
    value_type(
      m.basis_element(0,0)*v[0] +
      m.basis_element(1,0)*v[1] +
      m.basis_element(2,0)*v[2] +
      m.basis_element(3,0)),

    value_type(
      m.basis_element(0,1)*v[0] +
      m.basis_element(1,1)*v[1] +
      m.basis_element(2,1)*v[2] +
      m.basis_element(3,1)),

    value_type(
      m.basis_element(0,2)*v[0] +
      m.basis_element(1,2)*v[1] +
      m.basis_element(2,2)*v[2] +
      m.basis_element(3,2))
    );
}

template<class Sub1, class Sub2> inline auto
transform_vector_4D(
  const readable_matrix<Sub1>& m, const readable_vector<Sub2>& v
  ) -> temporary_of_t<Sub2>
{
  typedef basis_tag_of_t<Sub1>				tag;
  static_assert(tag::value != any_basis_c, "invalid matrix basis orientation");
  cml::check_size(m, int_c<4>(), int_c<4>());
  cml::check_size(v, int_c<4>());
  return detail::transform_vector_4D(m, v, tag());
}

template<class Sub1, class Sub2> inline auto
transform_point_4D(
  const readable_matrix<Sub1>& m, const readable_vector<Sub2>& v
  ) -> temporary_of_t<Sub2>
{
  typedef temporary_of_t<Sub2>				result_type;
  typedef value_type_trait_of_t<result_type>		value_type;

  cml::check_size(m, int_c<4>(), int_c<4>());
  cml::check_size(v, int_c<3>());

  /* 4D vector temporary: */
  vector<value_type, compiled<4>> h(
    value_type(
      m.basis_element(0,0)*v[0] +
      m.basis_element(1,0)*v[1] +
      m.basis_element(2,0)*v[2] +
      m.basis_element(3,0)),

    value_type(
      m.basis_element(0,1)*v[0] +
      m.basis_element(1,1)*v[1] +
      m.basis_element(2,1)*v[2] +
      m.basis_element(3,1)),

    value_type(
      m.basis_element(0,2)*v[0] +
      m.basis_element(1,2)*v[1] +
      m.basis_element(2,2)*v[2] +
      m.basis_element(3,2)),

    value_type(
      m.basis_element(0,3)*v[0] +
      m.basis_element(1,3)*v[1] +
      m.basis_element(2,3)*v[2] +
      m.basis_element(3,3))
    );

  /* Return projection: */
  return result_type(h[0] / h[3], h[1] / h[3], h[2] / h[3]);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
