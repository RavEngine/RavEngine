/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_QUATERNION_WRITABLE_QUATERNION_TPP
#error "quaternion/writable_quaternion.tpp not included correctly"
#endif

#include <cml/common/mpl/item_at.h>
#include <cml/vector/readable_vector.h>
#include <cml/quaternion/product.h>
#include <cml/quaternion/size_checking.h>

namespace cml {

/* Public methods: */

template<class DT> DT&
writable_quaternion<DT>::actual()
{
  return (DT&) *this;
}

template<class DT> auto
writable_quaternion<DT>::get(int i) -> mutable_value
{
  return this->actual().i_get(i);
}

template<class DT> template<class Other> DT&
writable_quaternion<DT>::put(int i, const Other& v) __CML_REF
{
  return this->actual().i_put(i,v);
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class Other> DT&&
writable_quaternion<DT>::put(int i, const Other& v) &&
{
  this->put(i,v);
  return (DT&&) *this;
}
#endif

template<class DT> auto
writable_quaternion<DT>::operator[](int i) -> mutable_value
{
  return this->get(i);
}

template<class DT> auto
writable_quaternion<DT>::w() -> mutable_value
{
  return this->get(order_type::W);
}

template<class DT> auto
writable_quaternion<DT>::x() -> mutable_value
{
  return this->get(order_type::X);
}

template<class DT> auto
writable_quaternion<DT>::y() -> mutable_value
{
  return this->get(order_type::Y);
}

template<class DT> auto
writable_quaternion<DT>::z() -> mutable_value
{
  return this->get(order_type::Z);
}


template<class DT> DT&
writable_quaternion<DT>::normalize() __CML_REF
{
  return this->operator/=(this->length());
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> DT&&
writable_quaternion<DT>::normalize() &&
{
  this->normalize();		// Forward to normalize &
  return (DT&&) *this;
}
#endif

template<class DT> DT&
writable_quaternion<DT>::zero() __CML_REF
{
  for(int i = 0; i < 4; ++ i) this->put(i, value_type(0));
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> DT&&
writable_quaternion<DT>::zero() &&
{
  this->zero();			// Forward to zero &
  return (DT&&) *this;
}
#endif

template<class DT> DT&
writable_quaternion<DT>::identity() __CML_REF
{
  this->put(W, value_type(1));
  this->put(X, value_type(0));
  this->put(Y, value_type(0));
  this->put(Z, value_type(0));
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> DT&&
writable_quaternion<DT>::identity() &&
{
  this->identity();		// Forward to identity &
  return (DT&&) *this;
}
#endif

template<class DT> DT&
writable_quaternion<DT>::conjugate() __CML_REF
{
  this->put(W,   this->get(W));
  this->put(X, - this->get(X));
  this->put(Y, - this->get(Y));
  this->put(Z, - this->get(Z));
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> DT&&
writable_quaternion<DT>::conjugate() &&
{
  this->conjugate();		// Forward to conjugate &
  return (DT&&) *this;
}
#endif

template<class DT> DT&
writable_quaternion<DT>::inverse() __CML_REF
{
  /* Cayley norm (squared length). */
  auto n = this->norm();

  /* The inverse is conjugate() / norm(): */
  this->put(W,   this->get(W) / n);
  this->put(X, - this->get(X) / n);
  this->put(Y, - this->get(Y) / n);
  this->put(Z, - this->get(Z) / n);
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> DT&&
writable_quaternion<DT>::inverse() &&
{
  this->inverse();		// Forward to inverse &
  return (DT&&) *this;
}
#endif

template<class DT> DT&
writable_quaternion<DT>::log() __CML_REF
{
  /* Shorthand: */
  typedef typename traits_type::element_traits		element_traits;

  /* The natural log of q is:
   *
   *   ln(|q|) + v/|v| * arccos(w / |q|)
   *
   * where v is the imaginary part of q and w is the real part:
   */
  auto lq = this->length();
  auto v = this->imaginary();
  auto lv = v.length();
  auto c = element_traits::acos(this->real() / lq) / lv;
  this->assign(c*v, element_traits::log(lq));
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> DT&&
writable_quaternion<DT>::log() &&
{
  this->log();			// Forward to log &
  return (DT&&) *this;
}
#endif

template<class DT> DT&
writable_quaternion<DT>::exp() __CML_REF
{
  /* Shorthand: */
  typedef typename traits_type::element_traits		element_traits;

  /* The exponential of q is:
   *
   *   exp(w) * (cos(|v|) + v/|v|*sin(|v|))
   *
   * where v is the imaginary part of q and w is the real part:
   */
  auto v = this->imaginary();
  auto lv = v.length();
  auto x = element_traits::exp(this->real());
  auto c = element_traits::sin(lv) / lv;
  this->assign(x*c*v, x*element_traits::cos(lv));
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> DT&&
writable_quaternion<DT>::exp() &&
{
  this->exp();			// Forward to exp &
  return (DT&&) *this;
}
#endif


#if 0
template<class DT> DT&
writable_quaternion<DT>::random(
  const_reference low, const_reference high
  ) __CML_REF
{
  typedef if_t<std::is_integral<value_type>::value
    , std::uniform_int_distribution<value_type>
    , std::uniform_real_distribution<value_type>>	distribution_type;

  std::default_random_engine gen(std::rand());
  distribution_type d(low, high);
  for(int i = 0; i < this->size(); ++ i) this->put(i, d(gen));
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> DT&&
writable_quaternion<DT>::random(const_reference low, const_reference high) &&
{
  this->random(low, high);
  return (DT&&) *this;
}
#endif
#endif

template<class DT>
template<class Sub, class E, enable_if_vector_t<Sub>*> auto
writable_quaternion<DT>::set(
  const readable_vector<Sub>& v, const E& s
  ) __CML_REF
-> enable_if_t<
  are_convertible<value_type, value_type_trait_of_t<Sub>, E>::value,
  DT&
  >
{
  return this->assign(v, s);
}

template<class DT>
template<class E, class Sub, enable_if_vector_t<Sub>*> auto
writable_quaternion<DT>::set(
  const E& s, const readable_vector<Sub>& v
  ) __CML_REF
-> enable_if_t<
  are_convertible<value_type, value_type_trait_of_t<Sub>, E>::value,
  DT&
  >
{
  return this->assign(v, s);
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS

template<class DT>
template<class Sub, class E, enable_if_vector_t<Sub>*> auto
writable_quaternion<DT>::set(
  const readable_vector<Sub>& v, const E& s
  ) &&
-> enable_if_t<
  are_convertible<value_type, value_type_trait_of_t<Sub>, E>::value,
  DT&&
  >
{
  this->assign(v, s);
  return (DT&&) *this;
}

template<class DT>
template<class E, class Sub, enable_if_vector_t<Sub>*> auto
writable_quaternion<DT>::set(
  const E& s, const readable_vector<Sub>& v
  ) &&
-> enable_if_t<
  are_convertible<value_type, value_type_trait_of_t<Sub>, E>::value,
  DT&&
  >
{
  this->assign(v, s);
  return (DT&&) *this;
}

#endif

template<class DT> template<class ODT> DT&
writable_quaternion<DT>::operator=(
  const readable_quaternion<ODT>& other
  ) __CML_REF
{
  return this->assign(other);
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class ODT> DT&&
writable_quaternion<DT>::operator=(const readable_quaternion<ODT>& other) &&
{
  this->operator=(other);
  return (DT&&) *this;
}
#endif

template<class DT>
template<class Array, enable_if_array_t<Array>*> DT&
writable_quaternion<DT>::operator=(const Array& array) __CML_REF
{
  return this->assign(array);
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT>
template<class Array, enable_if_array_t<Array>*> DT&&
writable_quaternion<DT>::operator=(const Array& array) &&
{
  this->operator=(array);
  return (DT&&) *this;
}
#endif

template<class DT> template<class Other> DT&
writable_quaternion<DT>::operator=(std::initializer_list<Other> l) __CML_REF
{
  return this->assign(l);
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class Other> DT&&
writable_quaternion<DT>::operator=(std::initializer_list<Other> l) &&
{
  return this->assign(l);
}
#endif

template<class DT> template<class ODT> DT&
writable_quaternion<DT>::operator+=(
  const readable_quaternion<ODT>& other
  ) __CML_REF
{
  typedef binary_plus_t<DT, ODT> op_type;
  for(int i = 0; i < 4; ++ i)
    this->put(i, op_type().apply(this->get(i), other.get(i)));
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class ODT> DT&&
writable_quaternion<DT>::operator+=(const readable_quaternion<ODT>& other) &&
{
  this->operator+=(other);
  return (DT&&) *this;
}
#endif

template<class DT> template<class ODT> DT&
writable_quaternion<DT>::operator-=(
  const readable_quaternion<ODT>& other
  ) __CML_REF
{
  typedef binary_minus_t<DT, ODT> op_type;
  for(int i = 0; i < 4; ++ i)
    this->put(i, op_type().apply(this->get(i), other.get(i)));
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class ODT> DT&&
writable_quaternion<DT>::operator-=(const readable_quaternion<ODT>& other) &&
{
  this->operator-=(other);
  return (DT&&) *this;
}
#endif

template<class DT> template<class ODT> DT&
writable_quaternion<DT>::operator*=(
  const readable_quaternion<ODT>& other
  ) __CML_REF
{
  return this->assign((*this) * other);
  /* Note: operator*() returns a temporary here. */
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class ODT> DT&&
writable_quaternion<DT>::operator*=(const readable_quaternion<ODT>& other) &&
{
  this->operator*=(other);
  return (DT&&) *this;
}
#endif

template<class DT>
template<class ScalarT, typename enable_if_convertible<
    typename quaternion_traits<DT>::value_type, ScalarT>::type*>
DT&
writable_quaternion<DT>::operator*=(const ScalarT& v) __CML_REF
{
  typedef binary_multiply_t<DT, ScalarT> op_type;
  for(int i = 0; i < 4; ++ i)
    this->put(i, op_type().apply(this->get(i), v));
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT>
template<class ScalarT, typename enable_if_convertible<
    typename quaternion_traits<DT>::value_type, ScalarT>::type*>
DT&&
writable_quaternion<DT>::operator*=(const ScalarT& v) &&
{
  this->operator*=(v);
  return (DT&&) *this;
}
#endif

template<class DT>
template<class ScalarT, typename enable_if_convertible<
    typename quaternion_traits<DT>::value_type, ScalarT>::type*>
DT&
writable_quaternion<DT>::operator/=(const ScalarT& v) __CML_REF
{
  typedef binary_divide_t<DT, ScalarT> op_type;
  for(int i = 0; i < 4; ++ i)
    this->put(i, op_type().apply(this->get(i), v));
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT>
template<class ScalarT, typename enable_if_convertible<
    typename quaternion_traits<DT>::value_type, ScalarT>::type*>
DT&&
writable_quaternion<DT>::operator/=(const ScalarT& v) &&
{
  this->operator/=(v);
  return (DT&&) *this;
}
#endif



/* Internal methods: */

template<class DT> template<class ODT> DT&
writable_quaternion<DT>::assign(const readable_quaternion<ODT>& other)
{
  this->put(W, other.get(W));
  this->put(X, other.get(X));
  this->put(Y, other.get(Y));
  this->put(Z, other.get(Z));
  return this->actual();
}

template<class DT> template<class ODT, class E0> DT&
writable_quaternion<DT>::assign(
  const readable_vector<ODT>& other, const E0& e0
  )
{
  cml::check_size(other, cml::int_c<3>());
  this->put(W, e0);
  this->put(X, other.get(0));
  this->put(Y, other.get(1));
  this->put(Z, other.get(2));
  return this->actual();
}

template<class DT>
template<class Array, enable_if_array_t<Array>*> DT&
writable_quaternion<DT>::assign(const Array& array)
{
  static const int N = array_size_of_c<Array>::value;
  static_assert(N == 4, "incorrect quaternion expression size");
  this->put(W, array[W]);
  this->put(X, array[X]);
  this->put(Y, array[Y]);
  this->put(Z, array[Z]);
  return this->actual();
}

template<class DT>
template<class Pointer, enable_if_pointer_t<Pointer>*> DT&
writable_quaternion<DT>::assign(const Pointer& array)
{
  this->put(W, array[W]);
  this->put(X, array[X]);
  this->put(Y, array[Y]);
  this->put(Z, array[Z]);
  return this->actual();
}

template<class DT>
template<class Array, class E0, enable_if_array_t<Array>*> DT&
writable_quaternion<DT>::assign(const Array& array, const E0& e0)
{
  static const int N = array_size_of_c<Array>::value;
  static_assert(N == 3, "incorrect quaternion expression size");
  this->put(W, e0);
  this->put(X, array[0]);
  this->put(Y, array[1]);
  this->put(Z, array[2]);
  return this->actual();
}

template<class DT> template<class Other> DT&
writable_quaternion<DT>::assign(const std::initializer_list<Other>& l)
{
#ifndef CML_NO_RUNTIME_QUATERNION_SIZE_CHECKS
  cml_require(l.size() == 4, quaternion_size_error, /**/);
#endif
  auto array = l.begin();
  this->put(W, array[W]);
  this->put(X, array[X]);
  this->put(Y, array[Y]);
  this->put(Z, array[Z]);
  return this->actual();
}

template<class DT>
template<class E0, class E1, class E2, class E3> DT&
writable_quaternion<DT>::assign_elements(
  const E0& e0, const E1& e1, const E2& e2, const E3& e3
  )
{
  this->put(W, item_at<W>(e0, e1, e2, e3));
  this->put(X, item_at<X>(e0, e1, e2, e3));
  this->put(Y, item_at<Y>(e0, e1, e2, e3));
  this->put(Z, item_at<Z>(e0, e1, e2, e3));
  return this->actual();
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
