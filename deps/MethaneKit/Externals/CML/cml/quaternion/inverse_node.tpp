/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_QUATERNION_INVERSE_NODE_TPP
#error "quaternion/inverse_node.tpp not included correctly"
#endif

#include <cml/quaternion/order_tags.h>

namespace cml {

/* inverse_node 'structors: */

template<class Sub>
inverse_node<Sub>::inverse_node(Sub sub)
: m_sub(std::move(sub)), m_inv_norm(value_type(1) / sub.norm())
{
}

template<class Sub>
inverse_node<Sub>::inverse_node(node_type&& other)
: m_sub(std::move(other.m_sub)), m_inv_norm(std::move(other.m_inv_norm))
{
}

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class Sub>
inverse_node<Sub>::inverse_node(const node_type& other)
: m_sub(other.m_sub), m_inv_norm(other.m_inv_norm)
{
}
#endif



/* Internal methods: */

/* readable_quaternion interface: */

template<class Sub> auto
inverse_node<Sub>::i_get(int i) const -> immutable_value
{
  typedef order_type_trait_of_t<sub_type> order_type;
  return this->m_inv_norm*((i == order_type::W)
    ? this->m_sub.get(i) : - this->m_sub.get(i));
  /* Note: W is either 0 (inverse_first) or 3 (real_first). */
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
