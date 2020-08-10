/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_QUATERNION_SCALAR_NODE_TPP
#error "quaternion/scalar_node.tpp not included correctly"
#endif

namespace cml {

/* quaternion_scalar_node 'structors: */

template<class Sub, class Scalar, class Op>
quaternion_scalar_node<Sub,Scalar,Op>::quaternion_scalar_node(
  Sub left, const right_type& right
  )
: m_left(std::move(left)), m_right(right)
{
}

template<class Sub, class Scalar, class Op>
quaternion_scalar_node<Sub,Scalar,Op>
::quaternion_scalar_node(node_type&& other)
: m_left(std::move(other.m_left)), m_right(std::move(other.m_right))
{
}

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class Sub, class Scalar, class Op>
quaternion_scalar_node<Sub,Scalar,Op>
::quaternion_scalar_node(const node_type& other)
: m_left(other.m_left), m_right(other.m_right)
{
}
#endif



/* Internal methods: */

/* readable_quaternion interface: */

template<class Sub, class Scalar, class Op> auto
quaternion_scalar_node<Sub,Scalar,Op>::i_get(int i) const -> immutable_value
{
  return Op().apply(this->m_left.get(i), this->m_right);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
