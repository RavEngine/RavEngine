/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_unary_node_h
#define	cml_matrix_unary_node_h

#include <cml/scalar/traits.h>
#include <cml/matrix/readable_matrix.h>

namespace cml {

template<class Sub, class Op> class matrix_unary_node;

/** matrix_unary_node<> traits. */
template<class Sub, class Op>
struct matrix_traits< matrix_unary_node<Sub,Op> >
{
  typedef matrix_unary_node<Sub,Op>			matrix_type;
  typedef Sub						sub_arg_type;
  typedef cml::unqualified_type_t<Sub>			sub_type;
  typedef matrix_traits<sub_type>			sub_traits;
  typedef scalar_traits<typename Op::result_type>	element_traits;
  typedef typename element_traits::value_type		value_type;
  typedef value_type					immutable_value;
  typedef typename sub_traits::storage_type		storage_type;
  typedef typename sub_traits::size_tag			size_tag;
  typedef typename sub_traits::basis_tag		basis_tag;
  typedef typename sub_traits::layout_tag		layout_tag;

  /* Propagate the rows from the subexpression: */
  static const int array_rows = sub_traits::array_rows;

  /* Propagate the columns from the subexpression: */
  static const int array_cols = sub_traits::array_cols;

  /** Constant containing the matrix basis enumeration value. */
  static const basis_kind matrix_basis = basis_tag::value;

  /** Constant containing the array layout enumeration value. */
  static const layout_kind array_layout = layout_tag::value;
};

/** Represents a unary matrix operation in an expression tree. */
template<class Sub, class Op>
class matrix_unary_node
: public readable_matrix< matrix_unary_node<Sub,Op> >
{
  public:

    typedef matrix_unary_node<Sub,Op>			node_type;
    typedef readable_matrix<node_type>			readable_type;
    typedef matrix_traits<node_type>			traits_type;
    typedef typename traits_type::sub_arg_type		sub_arg_type;
    typedef typename traits_type::sub_type		sub_type;
    typedef typename traits_type::element_traits	element_traits;
    typedef typename traits_type::value_type		value_type;
    typedef typename traits_type::immutable_value	immutable_value;
    typedef typename traits_type::storage_type		storage_type;
    typedef typename traits_type::size_tag		size_tag;
    typedef typename traits_type::basis_tag		basis_tag;
    typedef typename traits_type::layout_tag		layout_tag;


  public:

    /** Constant containing the number of rows. */
    static const int array_rows = traits_type::array_rows;

    /** Constant containing the number of columns. */
    static const int array_cols = traits_type::array_cols;

    /** Constant containing the matrix basis enumeration value. */
    static const basis_kind matrix_basis = traits_type::matrix_basis;

    /** Constant containing the array layout enumeration value. */
    static const layout_kind array_layout = traits_type::array_layout;


  public:

    /** Construct from the wrapped sub-expression.  @c sub must be an
     * lvalue reference or rvalue reference type.
     */
    explicit matrix_unary_node(Sub sub);

    /** Move constructor. */
    matrix_unary_node(node_type&& other);

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Copy constructor. */
    matrix_unary_node(const node_type& other);
#endif


  protected:

    /** @name readable_matrix Interface */
    /*@{*/

    friend readable_type;

    /** Return the row size of the matrix expression. */
    int i_rows() const;

    /** Return the column size of the matrix expression. */
    int i_cols() const;

    /** Apply the operator to element @c (i,j) of the subexpressions and
     * return the result.
     */
    immutable_value i_get(int i, int j) const;

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


  private:

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    // Not copy constructible.
    matrix_unary_node(const node_type&);
#endif

    // Not assignable.
    node_type& operator=(const node_type&);
};

} // namespace cml

#define __CML_MATRIX_UNARY_NODE_TPP
#include <cml/matrix/unary_node.tpp>
#undef __CML_MATRIX_UNARY_NODE_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
