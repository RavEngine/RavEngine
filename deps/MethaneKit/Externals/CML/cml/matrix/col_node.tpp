/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_COL_NODE_TPP
#error "matrix/col_node.tpp not included correctly"
#endif

namespace cml {

/* matrix_col_node 'structors: */

template<class Sub>
matrix_col_node<Sub,-1>::matrix_col_node(Sub sub, int col)
: m_sub(std::move(sub)), m_col(col)
{
  cml_require(col >= 0, std::invalid_argument, "col < 0");  
}

template<class Sub>
matrix_col_node<Sub,-1>::matrix_col_node(node_type&& other)
: m_sub(std::move(other.m_sub)), m_col(other.m_col)
{
}

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class Sub>
matrix_col_node<Sub,-1>::matrix_col_node(const node_type& other)
: m_sub(other.m_sub), m_col(other.m_col)
{
}
#endif



/* Internal methods: */

/* readable_vector interface: */

template<class Sub> int
matrix_col_node<Sub,-1>::i_size() const
{
  return this->m_sub.rows();
}

template<class Sub> auto
matrix_col_node<Sub,-1>::i_get(int i) const -> immutable_value
{
  return this->m_sub.get(i,this->m_col);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
