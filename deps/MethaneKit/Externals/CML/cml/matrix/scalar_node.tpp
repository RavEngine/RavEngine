/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_SCALAR_NODE_TPP
#error "matrix/scalar_node.tpp not included correctly"
#endif

namespace cml {

/* matrix_scalar_node 'structors: */

template<class Sub, class Scalar, class Op>
matrix_scalar_node<Sub,Scalar,Op>::matrix_scalar_node(
  Sub left, Scalar right
  )
: m_left(std::move(left)), m_right(std::move(right))
{
}

template<class Sub, class Scalar, class Op>
matrix_scalar_node<Sub,Scalar,Op>::matrix_scalar_node(node_type&& other)
: m_left(std::move(other.m_left)), m_right(std::move(other.m_right))
{
}

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class Sub, class Scalar, class Op>
matrix_scalar_node<Sub,Scalar,Op>::matrix_scalar_node(const node_type& other)
: m_left(other.m_left), m_right(other.m_right)
{
}
#endif



/* Internal methods: */

/* readable_matrix interface: */

template<class Sub, class Scalar, class Op> int
matrix_scalar_node<Sub,Scalar,Op>::i_rows() const
{
  return this->m_left.rows();
}

template<class Sub, class Scalar, class Op> int
matrix_scalar_node<Sub,Scalar,Op>::i_cols() const
{
  return this->m_left.cols();
}

template<class Sub, class Scalar, class Op> auto
matrix_scalar_node<Sub,Scalar,Op>::i_get(int i, int j) const -> immutable_value
{
  return Op().apply(this->m_left.get(i,j), this->m_right);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
