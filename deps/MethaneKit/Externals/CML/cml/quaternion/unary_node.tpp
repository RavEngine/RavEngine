/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_QUATERNION_UNARY_NODE_TPP
#error "quaternion/unary_node.tpp not included correctly"
#endif

namespace cml {

/* quaternion_unary_node 'structors: */

template<class Sub, class Op>
quaternion_unary_node<Sub,Op>::quaternion_unary_node(Sub sub)
: m_sub(std::move(sub))
{
}

template<class Sub, class Op>
quaternion_unary_node<Sub,Op>::quaternion_unary_node(node_type&& other)
: m_sub(std::move(other.m_sub))
{
}

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class Sub, class Op>
quaternion_unary_node<Sub,Op>::quaternion_unary_node(const node_type& other)
: m_sub(other.m_sub)
{
}
#endif



/* Internal methods: */

/* readable_quaternion interface: */

template<class Sub, class Op> auto
quaternion_unary_node<Sub,Op>::i_get(int i) const -> immutable_value
{
  return Op().apply(this->m_sub.get(i));
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
