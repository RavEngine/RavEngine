/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_binary_node_h
#define	cml_quaternion_binary_node_h

#include <cml/quaternion/readable_quaternion.h>
#include <cml/quaternion/promotion.h>

namespace cml {

template<class Sub1, class Sub2, class Op> class quaternion_binary_node;

/** quaternion_binary_node<> traits. */
template<class Sub1, class Sub2, class Op>
struct quaternion_traits< quaternion_binary_node<Sub1,Sub2,Op> >
{
  typedef quaternion_binary_node<Sub1,Sub2,Op>		quaternion_type;
  typedef Sub1						left_arg_type;
  typedef Sub2						right_arg_type;
  typedef cml::unqualified_type_t<Sub1>			left_type;
  typedef cml::unqualified_type_t<Sub2>			right_type;
  typedef quaternion_traits<left_type>			left_traits;
  typedef quaternion_traits<right_type>			right_traits;
  typedef scalar_traits<typename Op::result_type>	element_traits;
  typedef typename element_traits::value_type		value_type;
  typedef value_type					immutable_value;

  /* Determine the common storage type for the node, based on the storage
   * types of its subexpressions:
   */
  typedef quaternion_binary_storage_promote_t<
    storage_type_of_t<left_traits>,
    storage_type_of_t<right_traits>>			storage_type;
  typedef typename storage_type::size_tag		size_tag;
  static_assert(cml::is_fixed_size<storage_type>::value, "invalid size tag");

  /* Array size: */
  static const int array_size = storage_type::array_size;
  static_assert(array_size == 4, "invalid quaternion size");

  /* Determine the common order type: */
  typedef order_type_promote_t<
    order_type_of_t<left_traits>,
    order_type_of_t<right_traits>>			order_type;

  /* Determine the common cross type: */
  typedef cross_type_promote_t<
    cross_type_of_t<left_traits>,
    cross_type_of_t<right_traits>>			cross_type;
};

/** Represents a binary quaternion operation in an expression tree. */
template<class Sub1, class Sub2, class Op>
class quaternion_binary_node
: public readable_quaternion< quaternion_binary_node<Sub1,Sub2,Op> >
{
  public:

    typedef quaternion_binary_node<Sub1,Sub2,Op>	node_type;
    typedef readable_quaternion<node_type>		readable_type;
    typedef quaternion_traits<node_type>		traits_type;
    typedef typename traits_type::left_arg_type		left_arg_type;
    typedef typename traits_type::right_arg_type	right_arg_type;
    typedef typename traits_type::left_type		left_type;
    typedef typename traits_type::right_type		right_type;
    typedef typename traits_type::element_traits	element_traits;
    typedef typename traits_type::value_type		value_type;
    typedef typename traits_type::immutable_value	immutable_value;
    typedef typename traits_type::storage_type		storage_type;
    typedef typename traits_type::size_tag		size_tag;
    typedef typename traits_type::order_type		order_type;
    typedef typename traits_type::cross_type		cross_type;


  public:

    /** Constant containing the array size. */
    static const int array_size = traits_type::array_size;


  public:

    /** Construct from the wrapped sub-expressions.  Sub1 and Sub2 must be
     * lvalue reference or rvalue reference types.
     *
     * @throws incompatible_quaternion_size_error at run-time if either Sub1 or
     * Sub2 is a dynamically-sized quaternion, and sub1.size() != sub2.size().
     * If both Sub1 and Sub2 are fixed-size expressions, then the sizes are
     * checked at compile time.
     */
    quaternion_binary_node(Sub1 left, Sub2 right);

    /** Move constructor. */
    quaternion_binary_node(node_type&& other);

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Copy constructor. */
    quaternion_binary_node(const node_type& other);
#endif


  protected:

    /** @name readable_quaternion Interface */
    /*@{*/

    friend readable_type;

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
    quaternion_binary_node(const node_type&);
#endif

    // Not assignable.
    node_type& operator=(const node_type&);
};

} // namespace cml

#define __CML_QUATERNION_BINARY_NODE_TPP
#include <cml/quaternion/binary_node.tpp>
#undef __CML_QUATERNION_BINARY_NODE_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
