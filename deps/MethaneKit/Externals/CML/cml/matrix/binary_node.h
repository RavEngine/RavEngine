/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_binary_node_h
#define	cml_matrix_binary_node_h

#include <cml/matrix/readable_matrix.h>
#include <cml/matrix/promotion.h>

namespace cml {

template<class Sub1, class Sub2, class Op> class matrix_binary_node;

/** matrix_binary_node<> traits. */
template<class Sub1, class Sub2, class Op>
struct matrix_traits< matrix_binary_node<Sub1,Sub2,Op> >
{
  typedef matrix_binary_node<Sub1,Sub2,Op>		matrix_type;
  typedef Sub1						left_arg_type;
  typedef Sub2						right_arg_type;
  typedef cml::unqualified_type_t<Sub1>			left_type;
  typedef cml::unqualified_type_t<Sub2>			right_type;
  typedef matrix_traits<left_type>			left_traits;
  typedef matrix_traits<right_type>			right_traits;
  typedef scalar_traits<typename Op::result_type>	element_traits;
  typedef typename element_traits::value_type		value_type;
  typedef value_type					immutable_value;

  /* Determine the common storage type for the node, based on the storage
   * types of its subexpressions:
   */
  typedef matrix_binary_storage_promote_t<
    storage_type_of_t<left_traits>,
    storage_type_of_t<right_traits>>			storage_type;

  /* Traits and types for the storage: */
  typedef typename storage_type::size_tag		size_tag;

  /* Array rows: */
  static const int array_rows = storage_type::array_rows;

  /* Array cols: */
  static const int array_cols = storage_type::array_cols;

  /* Determine the common basis type: */
  typedef basis_tag_promote_t<
    basis_tag_of_t<left_traits>,
    basis_tag_of_t<right_traits>>			basis_tag;

  /* Determine the common layout type: */
  typedef layout_tag_promote_t<
    layout_tag_of_t<left_traits>,
    layout_tag_of_t<right_traits>>			layout_tag;

  /** Constant containing the matrix basis enumeration value. */
  static const basis_kind matrix_basis = basis_tag::value;

  /** Constant containing the array layout enumeration value. */
  static const layout_kind array_layout = layout_tag::value;
};

/** Represents a binary matrix operation in an expression tree. */
template<class Sub1, class Sub2, class Op>
class matrix_binary_node
: public readable_matrix< matrix_binary_node<Sub1,Sub2,Op> >
{
  public:

    typedef matrix_binary_node<Sub1,Sub2,Op>		node_type;
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

    /** Constant containing the number of rows. */
    static const int array_rows = traits_type::array_rows;

    /** Constant containing the number of columns. */
    static const int array_cols = traits_type::array_cols;

    /** Constant containing the array layout enumeration value. */
    static const layout_kind array_layout = traits_type::array_layout;

    /** Constant containing the matrix basis enumeration value. */
    static const basis_kind matrix_basis = traits_type::matrix_basis;


  public:

    /** Construct from the wrapped sub-expressions.  Sub1 and Sub2 must be
     * lvalue reference or rvalue reference types.
     *
     * @throws incompatible_matrix_sizes at run-time if either Sub1 or Sub2
     * is a dynamically-sized matrix, and sub1.size() != sub2.size().  If
     * both Sub1 and Sub2 are fixed-size expressions, then the sizes are
     * checked at compile time.
     */
    matrix_binary_node(Sub1 left, Sub2 right);

    /** Move constructor. */
    matrix_binary_node(node_type&& other);

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Copy constructor. */
    matrix_binary_node(const node_type& other);
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
     * stored as a copy if Sub1 is an rvalue reference (temporary), or by
     * const reference if Sub1 is an lvalue reference.
     */
    typedef cml::if_t<std::is_lvalue_reference<Sub1>::value,
	    const left_type&, left_type>		left_wrap_type;

    /** The type used to store the right subexpression.  The expression is
     * stored as a copy if Sub2 is an rvalue reference (temporary), or by
     * const reference if Sub2 is an lvalue reference.
     */
    typedef cml::if_t<std::is_lvalue_reference<Sub2>::value,
	    const right_type&, right_type>		right_wrap_type;


  protected:

    /** The wrapped left subexpression. */
    left_wrap_type		m_left;

    /** The wrapped right subexpression. */
    right_wrap_type		m_right;


  private:

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    // Not copy constructible.
    matrix_binary_node(const node_type&);
#endif

    // Not assignable.
    node_type& operator=(const node_type&);
};

} // namespace cml

#define __CML_MATRIX_BINARY_NODE_TPP
#include <cml/matrix/binary_node.tpp>
#undef __CML_MATRIX_BINARY_NODE_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
