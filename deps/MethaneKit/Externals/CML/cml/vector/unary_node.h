/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_unary_node_h
#define	cml_vector_unary_node_h

#include <cml/vector/readable_vector.h>

namespace cml {

template<class Sub, class Op> class vector_unary_node;

/** vector_unary_node<> traits. */
template<class Sub, class Op>
struct vector_traits< vector_unary_node<Sub,Op> >
{
  typedef vector_unary_node<Sub,Op>			vector_type;
  typedef Sub						sub_arg_type;
  typedef cml::unqualified_type_t<Sub>			sub_type;
  typedef vector_traits<sub_type>			sub_traits;
  typedef scalar_traits<typename Op::result_type>	element_traits;
  typedef typename element_traits::value_type		value_type;
  typedef value_type					immutable_value;
  typedef typename sub_traits::storage_type		storage_type;
  typedef typename sub_traits::size_tag			size_tag;

  /* Propagate the array size from the subexpression: */
  static const int array_size = sub_traits::array_size;
};

/** Represents a unary vector operation in an expression tree. */
template<class Sub, class Op>
class vector_unary_node
: public readable_vector< vector_unary_node<Sub,Op> >
{
  public:

    typedef vector_unary_node<Sub,Op>			node_type;
    typedef readable_vector<node_type>			readable_type;
    typedef vector_traits<node_type>			traits_type;
    typedef typename traits_type::sub_arg_type		sub_arg_type;
    typedef typename traits_type::sub_type		sub_type;
    typedef typename traits_type::element_traits	element_traits;
    typedef typename traits_type::value_type		value_type;
    typedef typename traits_type::immutable_value	immutable_value;
    typedef typename traits_type::storage_type		storage_type;
    typedef typename traits_type::size_tag		size_tag;


  public:

    /** The array size constant is the same as the subexpression. */
    static const int array_size = traits_type::array_size;


  public:

    /** Construct from the wrapped sub-expression.  @c sub must be an
     * lvalue reference or rvalue reference.
     */
    explicit vector_unary_node(Sub sub);

    /** Move constructor. */
    vector_unary_node(node_type&& other);

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Copy constructor. */
    vector_unary_node(const node_type& other);
#endif


  protected:

    /** @name readable_vector Interface */
    /*@{*/

    friend readable_type;

    /** Return the size of the vector expression. */
    int i_size() const;

    /** Apply the operator to element @c i and return the result. */
    immutable_value i_get(int i) const;

    /*@}*/


  protected:

    /** The type used to store the subexpression.  The expression is stored
     * as a copy if Sub is an rvalue reference (temporary), or by const
     * reference if Sub is an lvalue reference.
     */
    typedef cml::if_t<std::is_lvalue_reference<Sub>::value,
	    const sub_type&, sub_type>			sub_wrap_type;

    /** The wrapped subexpression. */
    sub_wrap_type		m_sub;


  private:

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    // Not copy constructible.
    vector_unary_node(const node_type&);
#endif

    // Not assignable.
    node_type& operator=(const node_type&);
};

} // namespace cml

#define __CML_VECTOR_UNARY_NODE_TPP
#include <cml/vector/unary_node.tpp>
#undef __CML_VECTOR_UNARY_NODE_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
