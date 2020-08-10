/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_QUATERNION_CONJUGATE_NODE_TPP
#error "quaternion/conjugate_node.tpp not included correctly"
#endif

#include <cml/quaternion/order_tags.h>

namespace cml {

/* conjugate_node 'structors: */

template<class Sub>
conjugate_node<Sub>::conjugate_node(Sub sub)
: m_sub(std::move(sub))
{
}

template<class Sub>
conjugate_node<Sub>::conjugate_node(node_type&& other)
: m_sub(std::move(other.m_sub))
{
}

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class Sub>
conjugate_node<Sub>::conjugate_node(const node_type& other)
: m_sub(other.m_sub)
{
}
#endif


/* Internal methods: */

/* readable_quaternion interface: */

template<class Sub> auto
conjugate_node<Sub>::i_get(int i) const -> immutable_value
{
  typedef order_type_trait_of_t<sub_type> order_type;
  return (i == order_type::W) ? this->m_sub.get(i) : - this->m_sub.get(i);
  /* Note: W is either 0 (conjugate_first) or 3 (real_first). */
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
