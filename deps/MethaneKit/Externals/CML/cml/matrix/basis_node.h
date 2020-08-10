/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_basis_node_h
#define	cml_matrix_basis_node_h

#include <cml/storage/type_util.h>
#include <cml/vector/readable_vector.h>
#include <cml/matrix/traits.h>

namespace cml {

template<class Sub, int I> class matrix_basis_node;

/** matrix_basis_node<> traits. */
template<class Sub, int I>
struct vector_traits< matrix_basis_node<Sub,I> >
{
  typedef matrix_basis_node<Sub,I>			vector_type;
  typedef Sub						sub_arg_type;
  typedef cml::unqualified_type_t<Sub>			sub_type;
  typedef matrix_traits<sub_type>			sub_traits;
  typedef typename sub_traits::element_traits		element_traits;
  typedef typename sub_traits::value_type		value_type;
  typedef typename sub_traits::immutable_value		immutable_value;

  /* Propagate the column count from the subexpression: */
  static const int array_size = sub_traits::array_cols;

  /* Deduce the vector storage type: */
  typedef typename sub_traits::storage_type		sub_storage_type;
  typedef typename sub_storage_type::unbound_type	sub_unbound_type;
  typedef resize_storage_t<
    sub_unbound_type, array_size>			resized_type;
  typedef rebind_vector_storage_t<resized_type>		storage_type;

  /* Take the size type from the storage type: */
  typedef typename storage_type::size_tag		size_tag;
};

/** Represents a read-only matrix basis vector, specified at run-time by
 * its index, as a node in an expression tree.
 */
template<class Sub>
class matrix_basis_node<Sub,-1>
: public readable_vector< matrix_basis_node<Sub,-1> >
{
  public:

    typedef matrix_basis_node<Sub,-1>			node_type;
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

    /** Constant containing the number of elements. */
    static const int array_size = traits_type::array_size;


  public:

    /** Construct from the wrapped sub-expression and the basis index.  @c
     * sub must be an lvalue reference or rvalue reference type.
     *
     * @throws std::invalid_argument if @c i < 0.
     */
    explicit matrix_basis_node(Sub sub, int i);

    /** Move constructor. */
    matrix_basis_node(node_type&& other);

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Copy constructor. */
    matrix_basis_node(const node_type& other);
#endif


  protected:

    /** @name readable_vector Interface */
    /*@{*/

    friend readable_type;

    /** Return the size of the vector expression. */
    int i_size() const;

    /** Return element @c (<basis index>,j) of the matrix. */
    immutable_value i_get(int j) const;

    /*@}*/


  protected:

    /** The type used to store the subexpression.  The expression is stored
     * as a copy if Sub is an rvalue reference (temporary), or by const
     * reference if Sub is an lvalue reference.
     */
    typedef cml::if_t<std::is_lvalue_reference<Sub>::value,
	    const sub_type&, sub_type>			sub_wrap_type;


  protected:

    /** The wrapped subexpression. */
    sub_wrap_type		m_sub;

    /** The basis index. */
    int				m_i;


  private:

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    // Not copy constructible.
    matrix_basis_node(const node_type&);
#endif

    // Not assignable.
    node_type& operator=(const node_type&);
};

} // namespace cml

#define __CML_MATRIX_BASIS_NODE_TPP
#include <cml/matrix/basis_node.tpp>
#undef __CML_MATRIX_BASIS_NODE_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
