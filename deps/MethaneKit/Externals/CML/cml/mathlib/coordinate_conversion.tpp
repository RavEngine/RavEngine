/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATHLIB_COORDINATE_CONVERSION_TPP
#error "mathlib/matrix/coordinate_conversion.tpp not included correctly"
#endif

#include <cml/scalar/functions.h>
#include <cml/common/mpl/are_convertible.h>
#include <cml/vector/size_checking.h>

namespace cml {

/* To Cartesian: */

template<class Sub, class E0, class E1> inline void
polar_to_cartesian(
  writable_vector<Sub>& v, E0 radius, E1 theta
  )
{
  typedef scalar_traits<E1>				theta_traits;

  static_assert(
    cml::are_convertible<value_type_trait_of_t<Sub>, E0, E1>::value,
    "incompatible scalar types");
  cml::check_size(v, int_c<2>());

  v[0] = theta_traits::cos(theta) * radius;
  v[1] = theta_traits::sin(theta) * radius;
}

template<class E, class Sub> inline void
polar_to_cartesian(
  E radius, E theta, writable_vector<Sub>& v
  )
{
  polar_to_cartesian(v, radius, theta);
}


template<class Sub, class E0, class E1, class E2> inline void
cylindrical_to_cartesian(
    writable_vector<Sub>& v, int axis, E0 radius, E1 theta, E2 height
    )
{
  typedef scalar_traits<E1>				theta_traits;

  static_assert(
    cml::are_convertible<value_type_trait_of_t<Sub>, E0, E1, E2>::value,
    "incompatible scalar types");
  cml_require(0 <= axis && axis <= 2,
    std::invalid_argument, "axis must be 0, 1, or 2");
  cml::check_size(v, int_c<3>());

  /* Make i = axis, and (j,k) equal to the other axis in cyclic order from
   * i:
   */
  int i, j, k;
  cml::cyclic_permutation(axis, i, j, k);

  /* Initialize the vector: */
  v[i] = height;
  v[j] = theta_traits::cos(theta) * radius;
  v[k] = theta_traits::sin(theta) * radius;
}

template<class E, class Sub> inline void
cylindrical_to_cartesian(
  E radius, E theta, E height, int axis, writable_vector<Sub>& v
  )
{
  cylindrical_to_cartesian(v, axis, radius, theta, height);
}


template<class Sub, class E0, class E1, class E2> inline void
spherical_to_cartesian(writable_vector<Sub>& v,
  int axis, LatitudeType type, E0 radius, E1 theta, E2 phi)
{
  typedef scalar_traits<E1>				theta_traits;
  typedef scalar_traits<E2>				phi_traits;

  static_assert(
    cml::are_convertible<value_type_trait_of_t<Sub>, E0, E1, E2>::value,
    "incompatible scalar types");
  cml_require(0 <= axis && axis <= 2,
    std::invalid_argument, "axis must be 0, 1, or 2");
  cml::check_size(v, int_c<3>());

  if(type == latitude) phi = constants<E2>::pi_over_2() - phi;

  auto sin_phi = phi_traits::sin(phi);
  auto cos_phi = phi_traits::cos(phi);
  auto sin_phi_r = sin_phi * radius;

  /* Make i = axis, and (j,k) equal to the other axis in cyclic order from
   * i:
   */
  int i, j, k;
  cml::cyclic_permutation(axis, i, j, k);

  /* Initialize the vector: */
  v[i] = cos_phi * radius;
  v[j] = sin_phi_r * theta_traits::cos(theta);
  v[k] = sin_phi_r * theta_traits::sin(theta);
}

template<class E, class Sub> inline void
spherical_to_cartesian(
  E radius, E theta, E phi,
  int axis, LatitudeType type, writable_vector<Sub>& v)
{
  spherical_to_cartesian(v, axis, type, radius, theta, phi);
}



/* From Cartesian: */

template<class Sub, class E0, class E1, class Tol> inline void
cartesian_to_polar(
  const readable_vector<Sub>& v, E0& radius, E1& theta, Tol tolerance
  )
{
  typedef value_type_trait_of_t<Sub>			value_type;
  typedef scalar_traits<value_type>			value_traits;

  static_assert(
    cml::are_convertible<value_type_trait_of_t<Sub>, E0, E1, Tol>::value,
    "incompatible scalar types");
  cml::check_size(v, int_c<2>());

  radius = v.length();
  theta = radius < tolerance
    ? E1(0) : E1(value_traits::atan2(v[1], v[0]));
}

template<class Sub, class E0, class E1> inline void
cartesian_to_polar(
  const readable_vector<Sub>& v, E0& radius, E1& theta
  )
{
  typedef value_type_trait_promote_t<Sub, E0, E1> tolerance_type;
  cartesian_to_polar(v, radius, theta,
    scalar_traits<tolerance_type>::sqrt_epsilon());
}


template<class Sub, class E0, class E1, class E2, class Tol> inline void
cartesian_to_cylindrical(const readable_vector<Sub>& v,
  int axis, E0& radius, E1& theta, E2& height, Tol tolerance
  )
{
  typedef value_type_trait_of_t<Sub>			value_type;
  typedef scalar_traits<value_type>			value_traits;

  static_assert(
    cml::are_convertible<value_type_trait_of_t<Sub>, E0, E1, E2, Tol>::value,
    "incompatible scalar types");
  cml_require(0 <= axis && axis <= 2,
    std::invalid_argument, "axis must be 0, 1, or 2");
  cml::check_size(v, int_c<3>());

  /* Make i = axis, and (j,k) equal to the other axis in cyclic order from
   * i:
   */
  int i, j, k;
  cml::cyclic_permutation(axis, i, j, k);

  /* Initialize return values; */
  height = E2(v[i]);
  radius = E0(cml::length(v[j], v[k]));
  theta = radius < tolerance
    ? E1(0) : E1(value_traits::atan2(v[k], v[j]));
}

template<class Sub, class E0, class E1, class E2> inline void
cartesian_to_cylindrical(
  const readable_vector<Sub>& v, int axis, E0& radius, E1& theta, E2& height
  )
{
  typedef value_type_trait_promote_t<Sub, E0, E1> tolerance_type;
  cartesian_to_cylindrical(v, axis, radius, theta, height,
    scalar_traits<tolerance_type>::sqrt_epsilon());
}

template<class Sub, class E> inline void
cartesian_to_cylindrical(const readable_vector<Sub>& v,
  E& radius, E& theta, E& height, int axis, E tolerance
  )
{
  cartesian_to_cylindrical(v, axis, radius, theta, height, tolerance);
}


template<class Sub, class E0, class E1, class E2, class Tol> inline void
cartesian_to_spherical(const readable_vector<Sub>& v,
  int axis, LatitudeType type, E0& radius, E1& theta, E2& phi,
  Tol tolerance
  )
{
  typedef value_type_trait_of_t<Sub>			value_type;
  typedef scalar_traits<value_type>			value_traits;

  static_assert(
    cml::are_convertible<value_type_trait_of_t<Sub>, E0, E1, E2, Tol>::value,
    "incompatible scalar types");
  cml_require(0 <= axis && axis <= 2,
    std::invalid_argument, "axis must be 0, 1, or 2");
  cml::check_size(v, int_c<3>());

  /* Make i = axis, and (j,k) equal to the other axis in cyclic order from
   * i:
   */
  int i, j, k;
  cml::cyclic_permutation(axis, i, j, k);

  auto len = cml::length(v[j], v[k]);
  theta = len < tolerance ? E1(0) : E1(value_traits::atan2(v[k], v[j]));
  radius = E0(cml::length(v[i], len));
  if(radius < tolerance) {
    phi = E2(0);
  } else {
    phi = E2(value_traits::atan2(len,v[i]));
    if(type == latitude) phi = constants<E2>::pi_over_2() - phi;
  }
}

template<class Sub, class E0, class E1, class E2> inline void
cartesian_to_spherical(const readable_vector<Sub>& v,
  int axis, LatitudeType type, E0& radius, E1& theta, E2& phi
  )
{
  typedef value_type_trait_promote_t<Sub, E0, E1> tolerance_type;
  cartesian_to_spherical(v, axis, type, radius, theta, phi,
    scalar_traits<tolerance_type>::sqrt_epsilon());
}

template<class Sub, class E> inline void
cartesian_to_spherical(const readable_vector<Sub>& v,
  E& radius, E& theta, E& phi, int axis, LatitudeType type, E tolerance
  )
{
  cartesian_to_spherical(v, axis, type, radius, theta, phi, tolerance);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
