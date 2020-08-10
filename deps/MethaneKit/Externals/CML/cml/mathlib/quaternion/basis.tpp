/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATHLIB_QUATERNION_BASIS_TPP
#error "mathlib/quaternion/basis.tpp not included correctly"
#endif

#include <cml/scalar/functions.h>

namespace cml {

template<class Sub> auto
quaternion_get_basis_vector(const readable_quaternion<Sub>& q, int i)
-> temporary_of_t<decltype(q.imaginary())>
{
  typedef value_type_trait_of_t<Sub>			value_type;
  typedef order_type_trait_of_t<Sub>			order_type;
  typedef temporary_of_t<decltype(q.imaginary())>	result_type;

  cml_require(0 <= i && i <= 2, std::invalid_argument, "invalid axis");

  int j = (i+1)%3, k = (i+2)%3;
  const auto W = order_type::W;
  const auto I = order_type::X + i;
  const auto J = order_type::X + j;
  const auto K = order_type::X + k;

  auto j2 = q[J] + q[J];
  auto k2 = q[K] + q[K];

  /* Done: */
  result_type basis;
  basis[i] = value_type(1) - q[J] * j2 - q[K] * k2;
  basis[j] = q[I] * j2 + q[W] * k2;
  basis[k] = q[I] * k2 - q[W] * j2;
  return basis;
}

template<class Sub> auto
quaternion_get_x_basis_vector(const readable_quaternion<Sub>& q)
-> temporary_of_t<decltype(q.imaginary())>
{
  return quaternion_get_basis_vector(q, 0);
}

template<class Sub> auto
quaternion_get_y_basis_vector(const readable_quaternion<Sub>& q)
-> temporary_of_t<decltype(q.imaginary())>
{
  return quaternion_get_basis_vector(q, 1);
}

template<class Sub> auto
quaternion_get_z_basis_vector(const readable_quaternion<Sub>& q)
-> temporary_of_t<decltype(q.imaginary())>
{
  return quaternion_get_basis_vector(q, 2);
}

template<class Sub,
  class XBasis, class YBasis, class ZBasis,
  enable_if_vector_t<XBasis>*,
  enable_if_vector_t<YBasis>*,
  enable_if_vector_t<ZBasis>*
> void quaternion_get_basis_vectors(
  const readable_quaternion<Sub>& q, XBasis& x, YBasis& y, ZBasis& z
  )
{
  x = quaternion_get_x_basis_vector(q);
  y = quaternion_get_y_basis_vector(q);
  z = quaternion_get_z_basis_vector(q);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
