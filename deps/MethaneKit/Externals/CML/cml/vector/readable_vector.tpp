/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_VECTOR_READABLE_VECTOR_TPP
#error "vector/readable_vector.tpp not included correctly"
#endif

#include <cml/scalar/functions.h>
#include <cml/scalar/binary_ops.h>
#include <cml/vector/scalar_node.h>
#include <cml/vector/subvector_node.h>
#include <cml/vector/size_checking.h>

namespace cml {

/* Public methods: */

template<class DT> const DT&
readable_vector<DT>::actual() const
{
  return (const DT&) *this;
}

template<class DT> int
readable_vector<DT>::size() const
{
  return this->actual().i_size();
}

template<class DT> auto
readable_vector<DT>::get(int i) const -> immutable_value
{
  return this->actual().i_get(i);
}

#ifdef CML_HAS_STRUCTURED_BINDINGS
template<class DT>
template<std::size_t I, enable_if_fixed_size<vector_traits<DT>>*> auto
readable_vector<DT>::get() const
-> immutable_value
{
  return this->actual().i_get(I);
}
#endif

template<class DT> auto
readable_vector<DT>::operator[](int i) const -> immutable_value
{
  return this->get(i);
}


template<class DT> auto
readable_vector<DT>::length_squared() const -> value_type
{
  cml::check_minimum_size(*this, cml::int_c<1>());
  auto accum = cml::sqr(this->get(0));
  for(int i = 1; i < this->size(); ++ i) accum += cml::sqr(this->get(i));
  return accum;
}

template<class DT> auto
readable_vector<DT>::length() const -> value_type
{
  return element_traits::sqrt(this->length_squared());
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> auto
readable_vector<DT>::normalize() const & -> vector_scalar_node<
	const DT&, value_type, op::binary_divide<value_type,value_type>>
{
  return vector_scalar_node<
    const DT&, value_type, op::binary_divide<value_type,value_type>>
    ((const DT&) *this, this->length());
}

template<class DT> auto
readable_vector<DT>::normalize() const && -> vector_scalar_node<
	DT&&, value_type, op::binary_divide<value_type,value_type>>
{
  return vector_scalar_node<
    DT&&, value_type, op::binary_divide<value_type,value_type>>
    ((DT&&) *this, this->length());
}
#else
template<class DT> auto
readable_vector<DT>::normalize() const -> vector_scalar_node<
	DT, value_type, op::binary_divide<value_type,value_type>>
{
  return vector_scalar_node<
    DT, value_type, op::binary_divide<value_type,value_type>>
    ((const DT&) *this, this->length());
}
#endif

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> auto
readable_vector<DT>::subvector(int i) const & -> subvector_node<const DT&>
{
  return subvector_node<const DT&>((const DT&) *this, i);
}

template<class DT> auto
readable_vector<DT>::subvector(int i) const && -> subvector_node<DT&&>
{
  return subvector_node<DT&&>((DT&&) *this, i);
}
#else
template<class DT> auto
readable_vector<DT>::subvector(int i) const -> subvector_node<DT>
{
  return subvector_node<DT>((const DT&) *this, i);
}
#endif

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
