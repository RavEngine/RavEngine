/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_VECTOR_DYNAMIC_EXTERNAL_TPP
#error "vector/dynamic_external.tpp not included correctly"
#endif

#include <cml/common/exception.h>

namespace cml {

/* dynamic_external 'structors: */

template<class E>
vector<E, external<>>::vector()
: m_data(0), m_size(0)
{
#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
  static_assert(false, "external vector default constructor not supported");
#endif
}

template<class E>
vector<E, external<>>::vector(pointer data, int size)
: m_data(data), m_size(size)
{
  cml_require(size >= 0, std::invalid_argument, "size < 0");
}

template<class E>
vector<E, external<>>::vector(int size, pointer data)
: m_data(data), m_size(size)
{
  cml_require(size >= 0, std::invalid_argument, "size < 0");
}

template<class E>
vector<E, external<>>::vector(const vector_type& other)
{
  this->m_data = other.m_data;
  this->m_size = other.m_size;
}

template<class E>
vector<E, external<>>::vector(vector_type&& other)
{
  this->m_data = other.m_data;
  this->m_size = other.m_size;
  other.m_data = nullptr;
  other.m_size = 0;
}



/* Public methods: */

template<class E> auto
vector<E, external<>>::data() -> pointer
{
  return this->m_data;
}

template<class E> auto
vector<E, external<>>::data() const -> const_pointer
{
  return this->m_data;
}

template<class E> auto
vector<E, external<>>::begin() const -> const_pointer
{
  return this->m_data;
}

template<class E> auto
vector<E, external<>>::end() const -> const_pointer
{
  return this->m_data + this->m_size;
}

template<class E> void
vector<E, external<>>::reset()
{
  this->m_size = 0;
  this->m_data = nullptr;
}


template<class E> auto
vector<E, external<>>::operator=(const vector_type& other) -> vector_type&
{
  return this->assign(other);
}

template<class E> auto
vector<E, external<>>::operator=(vector_type&& other) -> vector_type&
{
  this->m_data = other.m_data;
  this->m_size = other.m_size;
  other.m_data = nullptr;
  other.m_size = 0;
  return *this;
}



/* Internal methods: */

/* readable_vector interface: */

template<class E> int
vector<E, external<>>::i_size() const
{
  return this->m_size;
}

template<class E> auto
vector<E, external<>>::i_get(int i) const -> immutable_value
{
  return this->m_data[i];
}


/* writable_vector interface: */

template<class E> auto
vector<E, external<>>::i_get(int i) -> mutable_value
{
  return this->m_data[i];
}

template<class E> template<class Other> auto
vector<E, external<>>::i_put(int i, const Other& v) __CML_REF -> vector_type&
{
  this->m_data[i] = value_type(v);
  return *this;
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class E> template<class Other> auto
vector<E, external<>>::i_put(int i, const Other& v) && -> vector_type&&
{
  this->m_data[i] = value_type(v);
  return (vector_type&&) *this;
}
#endif

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
