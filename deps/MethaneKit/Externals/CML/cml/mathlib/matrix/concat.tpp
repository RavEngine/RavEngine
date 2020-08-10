/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATHLIB_MATRIX_CONCAT_TPP
#error "mathlib/matrix/concat.tpp not included correctly"
#endif

#include <cml/matrix/matrix_product.h>

namespace cml {
namespace detail {

/** Row-basis concatenation. */
template<class Sub1, class Sub2> inline auto
matrix_concat(
  const readable_matrix<Sub1>& m1, const readable_matrix<Sub2>& m2, row_basis
  )
-> matrix_inner_product_promote_t<Sub1, Sub2>
{
  return m2 * m1;
}

/** Column-basis concatenation. */
template<class Sub1, class Sub2> inline auto
matrix_concat(
  const readable_matrix<Sub1>& m1, const readable_matrix<Sub2>& m2, col_basis
  )
-> matrix_inner_product_promote_t<Sub1, Sub2>
{
  return m1 * m2;
}

} // namespace detail

template<class Sub1, class Sub2> inline auto
matrix_concat(
  const readable_matrix<Sub1>& m1, const readable_matrix<Sub2>& m2
  )
-> matrix_inner_product_promote_t<Sub1, Sub2>
{
  /* Use the promoted type to determine how to multiply: */
  typedef matrix_inner_product_promote_t<Sub1, Sub2>	concat_type;
  typedef basis_tag_of_t<concat_type>			tag;
  static_assert(tag::value != any_basis_c, "invalid matrix basis orientations");

  cml::check_square(m1);
  cml::check_square(m2);

  return detail::matrix_concat(m1, m2, tag());
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
