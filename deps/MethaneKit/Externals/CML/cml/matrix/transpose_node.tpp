/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_TRANSPOSE_NODE_TPP
#error "matrix/transpose_node.tpp not included correctly"
#endif

namespace cml {

/* matrix_transpose_node 'structors: */

template<class Sub>
matrix_transpose_node<Sub>::matrix_transpose_node(Sub sub)
: m_sub(std::move(sub))
{
}

template<class Sub>
matrix_transpose_node<Sub>::matrix_transpose_node(node_type&& other)
: m_sub(std::move(other.m_sub))
{
}

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class Sub>
matrix_transpose_node<Sub>::matrix_transpose_node(const node_type& other)
: m_sub(other.m_sub)
{
}
#endif



/* Internal methods: */

/* readable_matrix interface: */

template<class Sub> int
matrix_transpose_node<Sub>::i_rows() const
{
  return this->m_sub.cols();
}

template<class Sub> int
matrix_transpose_node<Sub>::i_cols() const
{
  return this->m_sub.rows();
}

template<class Sub> auto
matrix_transpose_node<Sub>::i_get(int i, int j) const -> immutable_value
{
  return this->m_sub.get(j,i);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
