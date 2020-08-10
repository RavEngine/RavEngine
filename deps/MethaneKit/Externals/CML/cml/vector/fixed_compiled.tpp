/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_VECTOR_FIXED_COMPILED_TPP
#error "vector/fixed_compiled.tpp not included correctly"
#endif

namespace cml {

/* fixed 'structors: */

template<class E, int S> template<class Sub>
vector<E, fixed<S>>::vector(const readable_vector<Sub>& sub)
{
  this->assign(sub);
}

template<class E, int S>
template<class Array, enable_if_array_t<Array>*>
vector<E, fixed<S>>::vector(const Array& array)
{
  this->assign(array);
}

template<class E, int S>
template<class Pointer, enable_if_pointer_t<Pointer>*>
vector<E, fixed<S>>::vector(const Pointer& array)
{
  this->assign(array);
}

template<class E, int S> template<class Other>
vector<E, fixed<S>>::vector(std::initializer_list<Other> l)
{
  this->assign(l);
}



/* Public methods: */

template<class E, int S> auto
vector<E, fixed<S>>::data() -> pointer
{
  return &this->m_data[0];
}

template<class E, int S> auto
vector<E, fixed<S>>::data() const -> const_pointer
{
  return &this->m_data[0];
}

template<class E, int S> auto
vector<E, fixed<S>>::begin() const -> const_pointer
{
  return &this->m_data[0];
}

template<class E, int S> auto
vector<E, fixed<S>>::end() const -> const_pointer
{
  return (&this->m_data[0]) + S;
}

template<class E, int S> auto
vector<E, fixed<S>>::operator=(const vector_type& other)
-> vector_type&
{
  return this->assign(other);
}

template<class E, int S> auto
vector<E, fixed<S>>::operator=(vector_type&& other)
-> vector_type&
{
  for(int i = 0; i < S; ++ i) this->m_data[i] = std::move(other.m_data[i]);
  return *this;
}



/* Internal methods: */

/* readable_vector interface: */

template<class E, int S> int
vector<E, fixed<S>>::i_size() const
{
  return S;
}

template<class E, int S> auto
vector<E, fixed<S>>::i_get(int i) const -> immutable_value
{
  return this->m_data[i];
}


/* writable_vector interface: */

template<class E, int S> auto
vector<E, fixed<S>>::i_get(int i) -> mutable_value
{
  return this->m_data[i];
}

template<class E, int S> template<class Other> auto
vector<E, fixed<S>>::i_put(int i, const Other& v) __CML_REF -> vector_type&
{
  this->m_data[i] = value_type(v);
  return *this;
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class E, int S> template<class Other> auto
vector<E, fixed<S>>::i_put(int i, const Other& v) && -> vector_type&&
{
  this->m_data[i] = value_type(v);
  return (vector_type&&) *this;
}
#endif

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
