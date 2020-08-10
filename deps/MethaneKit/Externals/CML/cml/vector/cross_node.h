/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_cross_node_h
#define	cml_vector_cross_node_h

#include <cml/vector/readable_vector.h>
#include <cml/vector/promotion.h>

namespace cml {

template<class Sub1, class Sub2> class vector_cross_node;

/** vector_cross_node<> traits. */
template<class Sub1, class Sub2>
struct vector_traits< vector_cross_node<Sub1,Sub2> >
{
  typedef vector_cross_node<Sub1,Sub2>			vector_type;
  typedef Sub1						left_arg_type;
  typedef Sub2						right_arg_type;
  typedef cml::unqualified_type_t<Sub1>			left_type;
  typedef cml::unqualified_type_t<Sub2>			right_type;
  typedef vector_traits<left_type>			left_traits;
  typedef vector_traits<right_type>			right_traits;

  typedef value_type_promote_t<left_traits, right_traits> element_type;
  typedef scalar_traits<element_type>			element_traits;
  typedef typename element_traits::value_type		value_type;
  typedef value_type					immutable_value;

  /* Determine the common storage type for the node, based on the storage
   * types of its subexpressions:
   */
  typedef vector_binary_storage_promote_t<
    storage_type_of_t<left_traits>,
    storage_type_of_t<right_traits>>			storage_type;

  /* Traits and types for the storage: */
  typedef typename storage_type::size_tag		size_tag;

  /* Array size: */
  static const int array_size = storage_type::array_size;
};

/** Represents a cross product in an expression tree. */
template<class Sub1, class Sub2>
class vector_cross_node
: public readable_vector< vector_cross_node<Sub1,Sub2> >
{
  public:

    typedef vector_cross_node<Sub1,Sub2>		node_type;
    typedef readable_vector<node_type>			readable_type;
    typedef vector_traits<node_type>			traits_type;
    typedef typename traits_type::left_arg_type		left_arg_type;
    typedef typename traits_type::right_arg_type	right_arg_type;
    typedef typename traits_type::left_type		left_type;
    typedef typename traits_type::right_type		right_type;
    typedef typename traits_type::element_traits	element_traits;
    typedef typename traits_type::value_type		value_type;
    typedef typename traits_type::immutable_value	immutable_value;
    typedef typename traits_type::storage_type		storage_type;
    typedef typename traits_type::size_tag		size_tag;


  public:

    /** Constant containing the array size. */
    static const int array_size = traits_type::array_size;


  public:

    /** Construct from the wrapped sub-expressions.  Sub1 and Sub2 must be
     * lvalue reference or rvalue reference types.
     *
     * @throws vector_size_error at run-time if either Sub1 or Sub2 is a
     * dynamically-sized vector, and sub1.size() != sub2.size() != 3.  If
     * both Sub1 and Sub2 are fixed-size expressions, then the sizes are
     * checked at compile time.
     */
    vector_cross_node(Sub1 left, Sub2 right);

    /** Move constructor. */
    vector_cross_node(node_type&& other);

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Copy constructor. */
    vector_cross_node(const node_type& other);
#endif


  protected:

    /** @name readable_vector Interface */
    /*@{*/

    friend readable_type;

    /** Return the size of the vector expression. */
    int i_size() const;

    /** Apply the operator to element @c i of the subexpressions and return
     * the result.
     */
    immutable_value i_get(int i) const;

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
    vector_cross_node(const node_type&);
#endif

    // Not assignable.
    node_type& operator=(const node_type&);
};

} // namespace cml

#define __CML_VECTOR_CROSS_NODE_TPP
#include <cml/vector/cross_node.tpp>
#undef __CML_VECTOR_CROSS_NODE_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
