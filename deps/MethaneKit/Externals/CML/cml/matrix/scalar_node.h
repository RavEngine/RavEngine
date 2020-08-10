/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_scalar_node_h
#define	cml_matrix_scalar_node_h

#include <cml/scalar/traits.h>
#include <cml/matrix/readable_matrix.h>

namespace cml {

template<class Sub, class Scalar, class Op> class matrix_scalar_node;

/** matrix_scalar_node<> traits. */
template<class Sub, class Scalar, class Op>
struct matrix_traits< matrix_scalar_node<Sub,Scalar,Op> >
{
  typedef matrix_scalar_node<Sub,Scalar,Op>		matrix_type;
  typedef Sub						left_arg_type;
  typedef Scalar 					right_arg_type;
  typedef cml::unqualified_type_t<Sub>			left_type;
  typedef cml::unqualified_type_t<Scalar>		right_type;
  typedef matrix_traits<left_type>			left_traits;
  typedef scalar_traits<typename Op::result_type>	element_traits;
  typedef typename element_traits::value_type		value_type;
  typedef value_type					immutable_value;
  typedef typename left_traits::storage_type		storage_type;
  typedef typename left_traits::size_tag		size_tag;
  typedef typename left_traits::basis_tag		basis_tag;
  typedef typename left_traits::layout_tag		layout_tag;

  /* Propagate the rows from the subexpression: */
  static const int array_rows = left_traits::array_rows;

  /* Propagate the columns from the subexpression: */
  static const int array_cols = left_traits::array_cols;

  /** Constant containing the matrix basis enumeration value. */
  static const basis_kind matrix_basis = basis_tag::value;

  /** Constant containing the array layout enumeration value. */
  static const layout_kind array_layout = layout_tag::value;
};

/** Represents a binary matrix operation, where one operand is a scalar
 * value, and the other is a matrix.
 */
template<class Sub, class Scalar, class Op>
class matrix_scalar_node
: public readable_matrix< matrix_scalar_node<Sub,Scalar,Op> >
{
  public:

    typedef matrix_scalar_node<Sub,Scalar,Op>		node_type;
    typedef readable_matrix<node_type>			readable_type;
    typedef matrix_traits<node_type>			traits_type;
    typedef typename traits_type::left_arg_type		left_arg_type;
    typedef typename traits_type::right_arg_type	right_arg_type;
    typedef typename traits_type::left_type		left_type;
    typedef typename traits_type::right_type		right_type;
    typedef typename traits_type::element_traits	element_traits;
    typedef typename traits_type::value_type		value_type;
    typedef typename traits_type::immutable_value	immutable_value;
    typedef typename traits_type::storage_type		storage_type;
    typedef typename traits_type::size_tag		size_tag;
    typedef typename traits_type::basis_tag		basis_tag;
    typedef typename traits_type::layout_tag		layout_tag;


  public:

    /** Take the array row size from the subexpression. */
    static const int array_rows = left_type::array_rows;

    /** Take the array column size from the subexpression. */
    static const int array_cols = left_type::array_cols;

    /** Constant containing the matrix basis enumeration value. */
    static const basis_kind matrix_basis = traits_type::matrix_basis;

    /** Constant containing the array layout enumeration value. */
    static const layout_kind array_layout = traits_type::array_layout;


  public:

    /** Construct from the wrapped sub-expression and the scalar to apply.
     * @c left and @c right must be lvalue or rvalue references.
     */
    matrix_scalar_node(Sub left, Scalar right);

    /** Move constructor. */
    matrix_scalar_node(node_type&& other);

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Copy constructor. */
    matrix_scalar_node(const node_type& other);
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

    /** The type used to store the left subexpression.  The expression is
     * stored as a copy if Sub is an rvalue reference (temporary), or by
     * const reference if Sub is an lvalue reference.
     */
    typedef cml::if_t<std::is_lvalue_reference<Sub>::value,
	    const left_type&, left_type>		left_wrap_type;


  protected:

    /** The matrix operand. */
    left_wrap_type		m_left;

    /** The scalar operand. */
    right_type			m_right;


  private:

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    // Not copy constructible.
    matrix_scalar_node(const node_type&);
#endif

    // Not assignable.
    node_type& operator=(const node_type&);
};

} // namespace cml

#define __CML_MATRIX_SCALAR_NODE_TPP
#include <cml/matrix/scalar_node.tpp>
#undef __CML_MATRIX_SCALAR_NODE_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
