/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_scalar_traits_h
#define	cml_scalar_traits_h

#include <cmath>
#include <limits>
#include <cml/common/compiler.h>
#include <cml/common/mpl/enable_if_arithmetic.h>
#include <cml/common/temporary.h>
#include <cml/common/traits.h>

namespace cml {
namespace detail {

/** Inheritable default scalar traits for integral types.  If
 * scalar_traits<> is specialized for a particular type T, T must be
 * default constructible and assignable.
 *
 * @note For the std:: numeric functions, @c Scalar is promoted to double
 * by default.
 *
 * @note default_integral_traits inherits from std::numeric_limits by default.
 */
template<typename Scalar> struct default_integral_traits
: std::numeric_limits<Scalar>
{
  typedef Scalar					value_type;
  typedef value_type*					pointer;
  typedef value_type&					reference;
  typedef value_type const*				const_pointer;
  typedef value_type const&				const_reference;
  typedef reference&					mutable_value;
  typedef const_reference				immutable_value;

  /** @name Basic Functions */
  /*@{*/

  static inline value_type fabs(const value_type& v) {
    return (value_type) std::fabs(double(v));
  }

  static inline value_type fmod(const value_type& v, const value_type& w) {
    return (value_type) std::fmod(double(v), double(w));
  }

  static CML_CONSTEXPR value_type sqrt(const value_type& v) {
    return (value_type) std::sqrt(double(v));
  }

  static inline value_type cos(const value_type& v) {
    return (value_type) std::cos(double(v));
  }

  static inline value_type sin(const value_type& v) {
    return (value_type) std::sin(double(v));
  }

  static inline value_type tan(const value_type& v) {
    return (value_type) std::tan(double(v));
  }

  static inline value_type acos(const value_type& v) {
    return (value_type) std::acos(double(v));
  }

  static inline value_type asin(const value_type& v) {
    return (value_type) std::asin(double(v));
  }

  static inline value_type atan(const value_type& v) {
    return (value_type) std::atan(double(v));
  }

  static inline value_type atan2(const value_type& x, const value_type& y) {
    return (value_type) std::atan2(double(x),double(y));
  }

  static inline value_type log(const value_type& v) {
    return (value_type) std::log(double(v));
  }

  static inline value_type exp(const value_type& v) {
    return (value_type) std::exp(double(v));
  }

  /*@}*/
};

/** Inheritable default scalar traits for floating point types.  If
 * scalar_traits<> is specialized for a particular type T, T must be
 * default constructible and assignable.
 *
 * @note default_scalar_traits inherits from std::numeric_limits by default.
 */
template<typename Scalar> struct default_floating_point_traits
: std::numeric_limits<Scalar>
{
  typedef Scalar					value_type;
  typedef value_type*					pointer;
  typedef value_type&					reference;
  typedef value_type const*				const_pointer;
  typedef value_type const&				const_reference;
  typedef reference&					mutable_value;
  typedef const_reference				immutable_value;

  /** @name Basic Functions */
  /*@{*/

  static inline value_type fabs(const value_type& v) {
    return std::fabs(v);
  }

  static inline value_type fmod(const value_type& v, const value_type& w) {
    return std::fmod(v, w);
  }

  static CML_CONSTEXPR value_type sqrt(const value_type& v) {
    return std::sqrt(v);
  }

  static inline value_type cos(const value_type& v) {
    return std::cos(v);
  }

  static inline value_type sin(const value_type& v) {
    return std::sin(v);
  }

  static inline value_type tan(const value_type& v) {
    return std::tan(v);
  }

  static inline value_type acos(const value_type& v) {
    return std::acos(v);
  }

  static inline value_type asin(const value_type& v) {
    return std::asin(v);
  }

  static inline value_type atan(const value_type& v) {
    return std::atan(v);
  }

  static inline value_type atan2(const value_type& x, const value_type& y) {
    return std::atan2(x,y);
  }

  static inline value_type log(const value_type& v) {
    return std::log(v);
  }

  static inline value_type exp(const value_type& v) {
    return std::exp(v);
  }

  /*@}*/
};

} // namespace detail


/** Specializable class aggregating scalar properties. */
template<typename Scalar, class Enable = void> struct scalar_traits;

/** Specialization of scalar traits for integral types. */
template<typename Scalar> struct scalar_traits<
Scalar, typename std::enable_if<std::is_integral<Scalar>::value>::type>
: detail::default_integral_traits<Scalar>
{
  /** Returns 0. */
  static CML_CONSTEXPR Scalar sqrt_epsilon() { return 0; }
};

/** Specialization of scalar_traits for floating-point types. */
template<typename Scalar> struct scalar_traits<
Scalar, typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
: detail::default_floating_point_traits<Scalar>
{
  /** Returns sqrt(numeric_limits<float>::epsilon()). */
  static CML_CONSTEXPR double sqrt_epsilon() {
    return detail::default_floating_point_traits<Scalar>::sqrt(
      std::numeric_limits<Scalar>::epsilon());
  }
};

/** Specialization of scalar_traits for float. */
template<> struct scalar_traits<float>
: detail::default_floating_point_traits<float>
{
  /** Returns a constant for sqrt(numeric_limits<float>::epsilon()). */
  static CML_CONSTEXPR float sqrt_epsilon() {
    return 3.452669831e-4f;		// 10 digits
  }
};

/** Specialization of scalar_traits for double. */
template<> struct scalar_traits<double>
: detail::default_floating_point_traits<double>
{
  /** Returns a constant for sqrt(numeric_limits<double>::epsilon()). */
  static CML_CONSTEXPR double sqrt_epsilon() {
    return 1.49011611938476563e-8;	// 18 digits
  }
};

/** Returns eps for type S. */
template<typename S> CML_CONSTEXPR S epsilon() {
  return scalar_traits<S>::epsilon();
}

/** Returns sqrt(eps) for type S. */
template<typename S> CML_CONSTEXPR S sqrt_epsilon() {
  return scalar_traits<S>::sqrt_epsilon();
}

/** traits_of for arithmetic scalars. */
template<class T> struct traits_of<T, enable_if_arithmetic_t<T>> {
  typedef scalar_traits<T>				type;
};

/** temporary_of for arithmetic scalars. */
template<class Scalar> struct temporary_of<
Scalar, cml::enable_if_arithmetic_t<Scalar>>
{
  typedef cml::value_type_trait_of_t<Scalar>		type;
};

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
