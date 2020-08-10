/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_VECTOR_WRITABLE_VECTOR_TPP
#error "vector/writable_vector.tpp not included correctly"
#endif

#include <random>
#include <cml/scalar/binary_ops.h>
#include <cml/vector/detail/check_or_resize.h>

namespace cml {
namespace detail {

/* Terminate the assignment recursion at the final element. */
template<int I, class Sub, class E0> inline void
assign_elements(writable_vector<Sub>& sub, cml::int_c<I>, const E0& e0)
{
  sub.put(I,e0);
}

/* Set element I of sub to e0, then assign the remainder of the elements
 * starting from I+1.
 */
template<int I, class Sub, class E0, class... Es> inline void
assign_elements(
  writable_vector<Sub>& sub, cml::int_c<I>, const E0& e0, const Es&... eN
  )
{
  sub.put(I,e0);
  assign_elements(sub, cml::int_c<I+1>(), eN...);
}



/* Set sub(i) to e0: */
template<class Sub, class E0> inline void
assign_elements(writable_vector<Sub>& sub, int i, const E0& e0)
{
  sub.put(i, e0);
}

/* Assign the elements of sub from eN starting from index i. */
template<class Sub, class E0, class... Es> inline void
assign_elements(
  writable_vector<Sub>& sub, int i, const E0& e0, const Es&... eN
  )
{
  sub.put(i, e0);
  assign_elements(sub, i+1, eN...);
}

} // namespace detail



/* Public methods: */

template<class DT> DT&
writable_vector<DT>::actual()
{
  return (DT&) *this;
}

template<class DT> auto
writable_vector<DT>::get(int i) -> mutable_value
{
  return this->actual().i_get(i);
}

#ifdef CML_HAS_STRUCTURED_BINDINGS
template<class DT>
template<std::size_t I, enable_if_fixed_size<vector_traits<DT>>*> auto
writable_vector<DT>::get() -> mutable_value
{
  return this->actual().i_get(I);
}
#endif

template<class DT> auto
writable_vector<DT>::operator[](int i) -> mutable_value
{
  return this->get(i);
}

template<class DT> template<class Other> DT&
writable_vector<DT>::put(int i, const Other& v) __CML_REF
{
  return this->actual().i_put(i,v);
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class Other> DT&&
writable_vector<DT>::put(int i, const Other& v) &&
{
  this->put(i,v);		// Forward to put(...) &
  return (DT&&) *this;
}
#endif


template<class DT> DT&
writable_vector<DT>::normalize() __CML_REF
{
  return this->operator/=(this->length());
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> DT&&
writable_vector<DT>::normalize() &&
{
  this->normalize();		// Forward to normalize &
  return (DT&&) *this;
}
#endif

template<class DT> DT&
writable_vector<DT>::zero() __CML_REF
{
  for(int i = 0; i < this->size(); ++ i) this->put(i,0);
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> DT&&
writable_vector<DT>::zero() &&
{
  this->zero();			// Forward to zero &
  return (DT&&) *this;
}
#endif

template<class DT> DT&
writable_vector<DT>::cardinal(int i) __CML_REF
{
  return this->zero().put(i,1);
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> DT&&
writable_vector<DT>::cardinal(int i) &&
{
  this->cardinal(i);		// Forward to cardinal &
  return (DT&&) *this;
}
#endif

template<class DT> template<class ODT> DT&
writable_vector<DT>::minimize(const readable_vector<ODT>& other) __CML_REF
{
  cml::check_same_size(*this, other);
  for(int i = 0; i < this->size(); ++ i) {
    this->put(i, std::min(this->get(i), value_type(other.get(i))));
  }
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class ODT> DT&&
writable_vector<DT>::minimize(const readable_vector<ODT>& other) &&
{
  this->minimize(other);		// Forward to minimize &
  return (DT&&) *this;
}
#endif

template<class DT> template<class ODT> DT&
writable_vector<DT>::maximize(const readable_vector<ODT>& other) __CML_REF
{
  cml::check_same_size(*this, other);
  for(int i = 0; i < this->size(); ++ i) {
    this->put(i, std::max(this->get(i), value_type(other.get(i))));
  }
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class ODT> DT&&
writable_vector<DT>::maximize(const readable_vector<ODT>& other) &&
{
  this->maximize(other);		// Forward to maximize &
  return (DT&&) *this;
}
#endif

template<class DT> DT&
writable_vector<DT>::random(
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
writable_vector<DT>::random(const_reference low, const_reference high) &&
{
  this->random(low, high);
  return (DT&&) *this;
}
#endif

template<class DT> DT&
writable_vector<DT>::fill(const_reference v) __CML_REF
{
  for(int i = 0; i < this->size(); ++ i) this->put(i, v);
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> DT&&
writable_vector<DT>::fill(const_reference v) &&
{
  this->fill(v);
  return (DT&&) *this;
}
#endif

template<class DT> template<class E0, class... Es> auto
writable_vector<DT>::set(const E0& e0, const Es&... eN) __CML_REF
-> enable_if_t<are_convertible<value_type, E0, Es...>::value, DT&>
{
  return this->assign_elements(e0, eN...);
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class E0, class... Es> auto
writable_vector<DT>::set(const E0& e0, const Es&... eN) &&
-> enable_if_t<are_convertible<value_type, E0, Es...>::value, DT&&>
{
  this->assign_elements(e0, eN...);
  return (DT&&) *this;
}
#endif

template<class DT> DT&
writable_vector<DT>::operator=(const writable_vector& other) __CML_REF
{
  return this->assign(other);
}

template<class DT> template<class ODT> DT&
writable_vector<DT>::operator=(const readable_vector<ODT>& other) __CML_REF
{
  return this->assign(other);
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class ODT> DT&&
writable_vector<DT>::operator=(const readable_vector<ODT>& other) &&
{
  this->operator=(other);
  return (DT&&) *this;
}
#endif

template<class DT>
template<class Array, enable_if_array_t<Array>*> DT&
writable_vector<DT>::operator=(const Array& array) __CML_REF
{
  return this->assign(array);
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT>
template<class Array, enable_if_array_t<Array>*> DT&&
writable_vector<DT>::operator=(const Array& array) &&
{
  this->operator=(array);
  return (DT&&) *this;
}
#endif

template<class DT> template<class Other> DT&
writable_vector<DT>::operator=(std::initializer_list<Other> l) __CML_REF
{
  return this->assign(l);
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class Other> DT&&
writable_vector<DT>::operator=(std::initializer_list<Other> l) &&
{
  return this->assign(l);
}
#endif

template<class DT> template<class ODT> DT&
writable_vector<DT>::operator+=(const readable_vector<ODT>& other) __CML_REF
{
  typedef binary_plus_t<DT, ODT> op_type;
  detail::check_or_resize(*this, other);
  for(int i = 0; i < this->size(); ++ i)
    this->put(i, op_type().apply(this->get(i), other.get(i)));
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class ODT> DT&&
writable_vector<DT>::operator+=(const readable_vector<ODT>& other) &&
{
  this->operator+=(other);
  return (DT&&) *this;
}
#endif

template<class DT> template<class ODT> DT&
writable_vector<DT>::operator-=(const readable_vector<ODT>& other) __CML_REF
{
  typedef binary_minus_t<DT, ODT> op_type;
  detail::check_or_resize(*this, other);
  for(int i = 0; i < this->size(); ++ i)
    this->put(i, op_type().apply(this->get(i), other.get(i)));
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class ODT> DT&&
writable_vector<DT>::operator-=(const readable_vector<ODT>& other) &&
{
  this->operator-=(other);
  return (DT&&) *this;
}
#endif

template<class DT>
template<class ScalarT, enable_if_convertible_t<
    typename vector_traits<DT>::value_type, ScalarT>*>
DT&
writable_vector<DT>::operator*=(const ScalarT& v) __CML_REF
{
  typedef binary_multiply_t<DT, ScalarT> op_type;
  for(int i = 0; i < this->size(); ++ i)
    this->put(i, op_type().apply(this->get(i), v));
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT>
template<class ScalarT, enable_if_convertible_t<
    typename vector_traits<DT>::value_type, ScalarT>*>
DT&&
writable_vector<DT>::operator*=(const ScalarT& v) &&
{
  this->operator*=(v);
  return (DT&&) *this;
}
#endif

template<class DT>
template<class ScalarT, enable_if_convertible_t<
    typename vector_traits<DT>::value_type, ScalarT>*>
DT&
writable_vector<DT>::operator/=(const ScalarT& v) __CML_REF
{
  typedef binary_divide_t<DT, ScalarT> op_type;
  for(int i = 0; i < this->size(); ++ i)
    this->put(i, op_type().apply(this->get(i), v));
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT>
template<class ScalarT, enable_if_convertible_t<
    typename vector_traits<DT>::value_type, ScalarT>*>
DT&&
writable_vector<DT>::operator/=(const ScalarT& v) &&
{
  this->operator/=(v);
  return (DT&&) *this;
}
#endif



/* Internal methods: */

template<class DT> template<class ODT> DT&
writable_vector<DT>::assign(const readable_vector<ODT>& other)
{
  detail::check_or_resize(*this, other);
  for(int i = 0; i < this->size(); ++ i) this->put(i, other.get(i));
  return this->actual();
}

template<class DT>
template<class Array, enable_if_array_t<Array>*> DT&
writable_vector<DT>::assign(const Array& array)
{
  static const int N = array_size_of_c<Array>::value;
  detail::check_or_resize(*this, N);
  for(int i = 0; i < N; ++ i) this->put(i, array[i]);
  return this->actual();
}

template<class DT>
template<class Pointer, enable_if_pointer_t<Pointer>*> DT&
writable_vector<DT>::assign(const Pointer& array)
{
  for(int i = 0; i < this->size(); ++ i) this->put(i, array[i]);
  return this->actual();
}

template<class DT> template<class Other> DT&
writable_vector<DT>::assign(const std::initializer_list<Other>& l)
{
  detail::check_or_resize(*this, l);
  int i = 0; for(Other v : l) { this->put(i ++, v); }
  return this->actual();
}

template<class DT> template<class ODT, class... Es> DT&
writable_vector<DT>::assign(
  const readable_vector<ODT>& other, const Es&... eN
  )
{
  detail::check_or_resize(*this, other, eN...);
  for(int i = 0; i < other.size(); ++ i) this->put(i, other.get(i));
  detail::assign_elements(*this, other.size(), eN...);
  return this->actual();
}

template<class DT> template<class... Es> DT&
writable_vector<DT>::assign_elements(const Es&... eN)
{
  static const int N = int(sizeof...(eN));
  detail::check_or_resize(*this, int_c<N>());
  detail::assign_elements(*this, int_c<0>(), eN...);
  return this->actual();
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
