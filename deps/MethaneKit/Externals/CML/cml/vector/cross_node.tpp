/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_VECTOR_CROSS_NODE_TPP
#error "vector/cross_node.tpp not included correctly"
#endif

#include <cml/vector/size_checking.h>

namespace cml {

/* vector_cross_node 'structors: */

template<class Sub1, class Sub2>
vector_cross_node<Sub1,Sub2>::vector_cross_node(Sub1 left, Sub2 right)
: m_left(std::move(left)), m_right(std::move(right))
{
  cml::check_size(left, cml::int_c<3>());
  cml::check_size(right, cml::int_c<3>());
  /* Note: this seems to be exception-safe since temporaries are stored by
   * value and references by reference.
   */
}

template<class Sub1, class Sub2>
vector_cross_node<Sub1,Sub2>::vector_cross_node(node_type&& other)
: m_left(std::move(other.m_left)), m_right(std::move(other.m_right))
{
}

template<class Sub1, class Sub2>
vector_cross_node<Sub1,Sub2>::vector_cross_node(const node_type& other)
: m_left(other.m_left), m_right(other.m_right)
{
}



/* Internal methods: */

/* readable_vector interface: */

template<class Sub1, class Sub2> int
vector_cross_node<Sub1,Sub2>::i_size() const
{
  return 3;
}

template<class Sub1, class Sub2> auto
vector_cross_node<Sub1,Sub2>::i_get(int i) const -> immutable_value
{
  int i0 = (i+1)%3, i1 = (i+2)%3;
  return immutable_value(
      this->m_left.get(i0)*this->m_right.get(i1)	// 1,2; 2,0; 0,1
    - this->m_left.get(i1)*this->m_right.get(i0))	// 2,1; 0,2; 1,0
    ;
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
