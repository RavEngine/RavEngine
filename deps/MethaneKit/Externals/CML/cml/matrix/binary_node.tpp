/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_BINARY_NODE_TPP
#error "matrix/binary_node.tpp not included correctly"
#endif

#include <cml/matrix/size_checking.h>

namespace cml {

/* matrix_binary_node 'structors: */

template<class Sub1, class Sub2, class Op>
matrix_binary_node<Sub1,Sub2,Op>::matrix_binary_node(Sub1 left, Sub2 right)
: m_left(std::move(left)), m_right(std::move(right))
{
  cml::check_same_size(this->m_left, this->m_right);
  /* Note: this seems to be exception-safe since temporaries are stored by
   * value and references by reference.
   */
}

template<class Sub1, class Sub2, class Op>
matrix_binary_node<Sub1,Sub2,Op>::matrix_binary_node(node_type&& other)
: m_left(std::move(other.m_left)), m_right(std::move(other.m_right))
{
}

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class Sub1, class Sub2, class Op>
matrix_binary_node<Sub1,Sub2,Op>::matrix_binary_node(const node_type& other)
: m_left(other.m_left), m_right(other.m_right)
{
}
#endif



/* Internal methods: */

/* readable_matrix interface: */

template<class Sub1, class Sub2, class Op> int
matrix_binary_node<Sub1,Sub2,Op>::i_rows() const
{
  return this->m_left.rows();
}

template<class Sub1, class Sub2, class Op> int
matrix_binary_node<Sub1,Sub2,Op>::i_cols() const
{
  return this->m_left.cols();
}

template<class Sub1, class Sub2, class Op> auto
matrix_binary_node<Sub1,Sub2,Op>::i_get(int i, int j) const -> immutable_value
{
  return Op().apply(this->m_left.get(i,j), this->m_right.get(i,j));
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
