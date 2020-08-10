/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_ROW_NODE_TPP
#error "matrix/row_node.tpp not included correctly"
#endif

namespace cml {

/* matrix_row_node 'structors: */

template<class Sub>
matrix_row_node<Sub,-1>::matrix_row_node(Sub sub, int row)
: m_sub(std::move(sub)), m_row(row)
{
  cml_require(row >= 0, std::invalid_argument, "row < 0");  
}

template<class Sub>
matrix_row_node<Sub,-1>::matrix_row_node(node_type&& other)
: m_sub(std::move(other.m_sub)), m_row(other.m_row)
{
}

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class Sub>
matrix_row_node<Sub,-1>::matrix_row_node(const node_type& other)
: m_sub(other.m_sub), m_row(other.m_row)
{
}
#endif



/* Internal methods: */

/* readable_vector interface: */

template<class Sub> int
matrix_row_node<Sub,-1>::i_size() const
{
  return this->m_sub.cols();
}

template<class Sub> auto
matrix_row_node<Sub,-1>::i_get(int j) const -> immutable_value
{
  return this->m_sub.get(this->m_row,j);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
