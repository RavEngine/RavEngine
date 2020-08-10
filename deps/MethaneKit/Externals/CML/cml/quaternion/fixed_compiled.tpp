/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_QUATERNION_FIXED_COMPILED_TPP
#error "quaternion/fixed_compiled.tpp not included correctly"
#endif

namespace cml {

/* fixed 'structors: */

template<class E, class O, class C> template<class Sub>
quaternion<E, fixed<>, O, C>::quaternion(
  const readable_quaternion<Sub>& sub
  )
{
  this->assign(sub);
}

template<class E, class O, class C>
template<class Array, enable_if_array_t<Array>*>
quaternion<E, fixed<>, O, C>::quaternion(const Array& array)
{
  this->assign(array);
}

template<class E, class O, class C>
template<class Pointer, enable_if_pointer_t<Pointer>*>
quaternion<E, fixed<>, O, C>::quaternion(const Pointer& array)
{
  this->assign(array);
}

template<class E, class O, class C>
template<class E0, class Array, enable_if_array_t<Array>*>
quaternion<E, fixed<>, O, C>::quaternion(const E0& e0, const Array& array)
{
  this->assign(array, e0);
}

template<class E, class O, class C>
template<class Array, class E1, enable_if_array_t<Array>*>
quaternion<E, fixed<>, O, C>::quaternion(const Array& array, const E1& e1)
{
  this->assign(array, e1);
}

template<class E, class O, class C> template<class Other>
quaternion<E, fixed<>, O, C>::quaternion(std::initializer_list<Other> l)
{
  this->assign(l);
}



/* Public methods: */

template<class E, class O, class C> int
quaternion<E, fixed<>, O, C>::size() const
{
  return 4;
}

template<class E, class O, class C> auto
quaternion<E, fixed<>, O, C>::data() -> pointer
{
  return &this->m_data[0];
}

template<class E, class O, class C> auto
quaternion<E, fixed<>, O, C>::data() const -> const_pointer
{
  return &this->m_data[0];
}

template<class E, class O, class C> auto
quaternion<E, fixed<>, O, C>::begin() const -> const_pointer
{
  return &this->m_data[0];
}

template<class E, class O, class C> auto
quaternion<E, fixed<>, O, C>::end() const -> const_pointer
{
  return (&this->m_data[0]) + 4;
}

template<class E, class O, class C> auto
quaternion<E, fixed<>, O, C>::operator=(const quaternion_type& other)
-> quaternion_type&
{
  return this->assign(other);
}

template<class E, class O, class C> auto
quaternion<E, fixed<>, O, C>::operator=(quaternion_type&& other)
-> quaternion_type&
{
  this->m_data[W] = std::move(other.m_data[W]);
  this->m_data[X] = std::move(other.m_data[X]);
  this->m_data[Y] = std::move(other.m_data[Y]);
  this->m_data[Z] = std::move(other.m_data[Z]);
  return *this;
}



/* Internal methods: */

/* readable_quaternion interface: */

template<class E, class O, class C> auto
quaternion<E, fixed<>, O, C>::i_get(int i) const -> immutable_value
{
  return this->m_data[i];
}


/* writable_quaternion interface: */

template<class E, class O, class C> auto
quaternion<E, fixed<>, O, C>::i_get(int i) -> mutable_value
{
  return this->m_data[i];
}

template<class E, class O, class C> template<class Other> auto
quaternion<E, fixed<>, O, C>::i_put(
  int i, const Other& v
  ) __CML_REF -> quaternion_type&
{
  this->m_data[i] = value_type(v);
  return *this;
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class E, class O, class C> template<class Other> auto
quaternion<E, fixed<>, O, C>::i_put(
  int i, const Other& v
  ) && -> quaternion_type&&
{
  this->m_data[i] = value_type(v);
  return (quaternion_type&&) *this;
}
#endif

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
