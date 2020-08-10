/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_VECTOR_FIXED_CONST_EXTERNAL_TPP
#error "vector/fixed_const_external.tpp not included correctly"
#endif

namespace cml {

/* fixed_external 'structors: */

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class E, int S>
vector<const E, external<S>>::vector()
: m_data(0)
{
}
#endif

template<class E, int S>
vector<const E, external<S>>::vector(const_pointer data)
: m_data(data)
{
}

template<class E, int S>
vector<const E, external<S>>::vector(vector_type&& other)
{
  this->m_data = other.m_data;
  other.m_data = nullptr;
}



/* Public methods: */

template<class E, int S> auto
vector<const E, external<S>>::data() const -> const_pointer
{
  return this->m_data;
}

template<class E, int S> auto
vector<const E, external<S>>::begin() const -> const_pointer
{
  return this->m_data;
}

template<class E, int S> auto
vector<const E, external<S>>::end() const -> const_pointer
{
  return this->m_data + S;
}



/* Internal methods: */

/* readable_vector interface: */

template<class E, int S> int
vector<const E, external<S>>::i_size() const
{
  return S;
}

template<class E, int S> auto
vector<const E, external<S>>::i_get(int i) const -> immutable_value
{
  return this->m_data[i];
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
