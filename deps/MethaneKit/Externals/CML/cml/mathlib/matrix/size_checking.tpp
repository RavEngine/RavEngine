/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATHLIB_MATRIX_SIZE_CHECKING_TPP
#error "mathlib/matrix/size_checking.tpp not included correctly"
#endif

#include <cml/matrix/size_checking.h>

namespace cml {
namespace detail {

/* No-op matrix size checking. */
template<class Sub> inline void
check_affine_2D(const readable_matrix<Sub>&, any_basis)
{
}

/* Size checking for a row-basis matrix: */
template<class Sub> inline void
check_affine_2D(const readable_matrix<Sub>& m, row_basis)
{
  cml::check_minimum_size(m, cml::int_c<3>(), cml::int_c<2>());
}

/* Size checking for a column-basis matrix: */
template<class Sub> inline void
check_affine_2D(const readable_matrix<Sub>& m, col_basis)
{
  cml::check_minimum_size(m, cml::int_c<2>(), cml::int_c<3>());
}


/* No-op matrix size checking. */
template<class Sub> inline void
check_affine_3D(const readable_matrix<Sub>&, any_basis)
{
}

/* Size checking for a row-basis matrix: */
template<class Sub> inline void
check_affine_3D(const readable_matrix<Sub>& m, row_basis)
{
  cml::check_minimum_size(m, cml::int_c<4>(), cml::int_c<3>());
}

/* Size checking for a column-basis matrix: */
template<class Sub> inline void
check_affine_3D(const readable_matrix<Sub>& m, col_basis)
{
  cml::check_minimum_size(m, cml::int_c<3>(), cml::int_c<4>());
}


/* Compile-time affine matrix size checking: */
template<class Sub> inline void
check_affine(const readable_matrix<Sub>&, fixed_size_tag)
{
  typedef matrix_traits<Sub>				traits;

  /* If m is row basis, then rows == cols or rows == cols+1.  Otherwise, m
   * is column basis, and cols == rows or cols == rows+1:
   */
  static const int M = is_row_basis<traits>::value
    ? traits::array_rows : traits::array_cols;
  static const int N = is_row_basis<traits>::value
    ? traits::array_cols : traits::array_rows;
  static_assert(M == N || M == N+1, "incorrect affine matrix size");
}

/* Run-time affine matrix size checking: */
template<class Sub> inline void
check_affine(const readable_matrix<Sub>& m, dynamic_size_tag)
{
  typedef matrix_traits<Sub>				traits;

  /* If m is row basis, then rows == cols or rows == cols+1.  Otherwise, m
   * is column basis, and cols == rows or cols == rows+1:
   */
  int M = is_row_basis<traits>::value ? m.rows() : m.cols();
  int N = is_row_basis<traits>::value ? m.cols() : m.rows();
  cml_require(M == N || M == N+1, affine_matrix_size_error, /**/);
}

} // namespace detail


template<class Sub> inline void
check_affine_2D(const readable_matrix<Sub>& m)
{
  typedef basis_tag_of_t<Sub> tag;
  detail::check_affine_2D(m, tag());
}

template<class Sub> inline void
check_affine_3D(const readable_matrix<Sub>& m)
{
  typedef basis_tag_of_t<Sub> tag;
  detail::check_affine_3D(m, tag());
}

template<class Sub> inline void
check_affine(const readable_matrix<Sub>& m)
{
  typedef size_tag_of_t<Sub> size_tag;
  static_assert(!is_any_basis<Sub>::value, "row_basis or col_basis required");
  detail::check_affine(m, size_tag());
}

template<class Sub> inline void
check_linear_2D(const readable_matrix<Sub>& m)
{
  cml::check_minimum_size(m, cml::int_c<2>(), cml::int_c<2>());
}

template<class Sub> inline void
check_linear_3D(const readable_matrix<Sub>& m)
{
  cml::check_minimum_size(m, cml::int_c<3>(), cml::int_c<3>());
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
