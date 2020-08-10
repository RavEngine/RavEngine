/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_QUATERNION_BINARY_NODE_TPP
#error "quaternion/binary_node.tpp not included correctly"
#endif

namespace cml {

/* quaternion_binary_node 'structors: */

template<class Sub1, class Sub2, class Op>
quaternion_binary_node<Sub1,Sub2,Op>::quaternion_binary_node(Sub1 left, Sub2 right)
: m_left(std::move(left)), m_right(std::move(right))
{
}

template<class Sub1, class Sub2, class Op>
quaternion_binary_node<Sub1,Sub2,Op>::quaternion_binary_node(node_type&& other)
: m_left(std::move(other.m_left)), m_right(std::move(other.m_right))
{
}

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class Sub1, class Sub2, class Op>
quaternion_binary_node<Sub1,Sub2,Op>
::quaternion_binary_node(const node_type& other)
: m_left(other.m_left), m_right(other.m_right)
{
}
#endif


/* Internal methods: */

/* readable_quaternion interface: */

template<class Sub1, class Sub2, class Op> auto
quaternion_binary_node<Sub1,Sub2,Op>::i_get(int i) const -> immutable_value
{
  return Op().apply(this->m_left.get(i), this->m_right.get(i));
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
