/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_BASIS_NODE_TPP
#error "matrix/basis_node.tpp not included correctly"
#endif

namespace cml {

/* matrix_basis_node 'structors: */

template<class Sub>
matrix_basis_node<Sub,-1>::matrix_basis_node(Sub sub, int i)
: m_sub(std::move(sub)), m_i(i)
{
  cml_require(i >= 0, std::invalid_argument, "i < 0");  
}

template<class Sub>
matrix_basis_node<Sub,-1>::matrix_basis_node(node_type&& other)
: m_sub(std::move(other.m_sub)), m_i(other.m_i)
{
}

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class Sub>
matrix_basis_node<Sub,-1>::matrix_basis_node(const node_type& other)
: m_sub(other.m_sub), m_i(other.m_i)
{
}
#endif



/* Internal methods: */

/* readable_vector interface: */

template<class Sub> int
matrix_basis_node<Sub,-1>::i_size() const
{
  return this->m_sub.basis_size();
}

template<class Sub> auto
matrix_basis_node<Sub,-1>::i_get(int j) const -> immutable_value
{
  return this->m_sub.basis_element(this->m_i,j);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
