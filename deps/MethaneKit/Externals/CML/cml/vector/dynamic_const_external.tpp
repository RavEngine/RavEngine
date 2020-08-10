/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_VECTOR_DYNAMIC_CONST_EXTERNAL_TPP
#error "vector/dynamic_const_external.tpp not included correctly"
#endif

#include <cml/common/exception.h>

namespace cml {

/* dynamic_const_external 'structors: */

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class E>
vector<const E, external<>>::vector()
: m_data(0), m_size(0)
{
}
#endif

template<class E>
vector<const E, external<>>::vector(const_pointer data, int size)
: m_data(data), m_size(size)
{
  cml_require(size >= 0, std::invalid_argument, "size < 0");
}

template<class E>
vector<const E, external<>>::vector(int size, const_pointer data)
: m_data(data), m_size(size)
{
  cml_require(size >= 0, std::invalid_argument, "size < 0");
}

template<class E>
vector<const E, external<>>::vector(vector_type&& other)
{
  this->m_data = other.m_data;
  this->m_size = other.m_size;
  other.m_data = nullptr;
  other.m_size = 0;
}



/* Public methods: */

template<class E> auto
vector<const E, external<>>::data() const -> const_pointer
{
  return this->m_data;
}

template<class E> auto
vector<const E, external<>>::begin() const -> const_pointer
{
  return this->m_data;
}

template<class E> auto
vector<const E, external<>>::end() const -> const_pointer
{
  return this->m_data + this->m_size;
}

template<class E> void
vector<const E, external<>>::reset()
{
  this->m_size = 0;
  this->m_data = nullptr;
}



/* Internal methods: */

/* readable_vector interface: */

template<class E> int
vector<const E, external<>>::i_size() const
{
  return this->m_size;
}

template<class E> auto
vector<const E, external<>>::i_get(int i) const -> immutable_value
{
  return this->m_data[i];
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
