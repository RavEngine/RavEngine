/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_unary_node_h
#define	cml_quaternion_unary_node_h

#include <cml/common/size_tags.h>
#include <cml/quaternion/readable_quaternion.h>

namespace cml {

template<class Sub, class Op> class quaternion_unary_node;

/** quaternion_unary_node<> traits. */
template<class Sub, class Op>
struct quaternion_traits< quaternion_unary_node<Sub,Op> >
{
  /* Figure out the basic types of Sub: */
  typedef quaternion_unary_node<Sub,Op>			quaternion_type;
  typedef Sub						sub_arg_type;
  typedef cml::unqualified_type_t<Sub>			sub_type;
  typedef quaternion_traits<sub_type>			sub_traits;
  typedef scalar_traits<typename Op::result_type>	element_traits;
  typedef typename element_traits::value_type		value_type;
  typedef value_type					immutable_value;
  typedef typename sub_traits::storage_type		storage_type;

  typedef typename sub_traits::size_tag			size_tag;
  static_assert(cml::is_fixed_size<storage_type>::value, "invalid size tag");

  /* Array size: */
  static const int array_size = storage_type::array_size;
  static_assert(array_size == 4, "invalid quaternion size");

  /* Order and cross taken from the sub-expression: */
  typedef typename sub_traits::order_type		order_type;
  typedef typename sub_traits::cross_type		cross_type;
};

/** Represents a unary quaternion operation. */
template<class Sub, class Op>
class quaternion_unary_node
: public readable_quaternion< quaternion_unary_node<Sub,Op> >
{
  public:

    typedef quaternion_unary_node<Sub,Op>		node_type;
    typedef readable_quaternion<node_type>		readable_type;
    typedef quaternion_traits<node_type>		traits_type;
    typedef typename traits_type::sub_arg_type		sub_arg_type;
    typedef typename traits_type::sub_type		sub_type;
    typedef typename traits_type::element_traits	element_traits;
    typedef typename traits_type::value_type		value_type;
    typedef typename traits_type::immutable_value	immutable_value;
    typedef typename traits_type::storage_type		storage_type;
    typedef typename traits_type::size_tag		size_tag;
    typedef typename traits_type::order_type		order_type;
    typedef typename traits_type::cross_type		cross_type;


  public:

    /** The array size constant is the same as the subexpression. */
    static const int array_size = traits_type::array_size;


  public:

    /** Construct from the wrapped sub-expression.  @c sub must be an
     * lvalue reference or rvalue reference.
     */
    quaternion_unary_node(Sub sub);

    /** Move constructor. */
    quaternion_unary_node(node_type&& other);

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Copy constructor. */
    quaternion_unary_node(const node_type& other);
#endif


  protected:

    /** @name readable_quaternion Interface */
    /*@{*/

    friend readable_type;

    /** Apply the unary operator to element @c i of the subexpression and
     * return the result.
     */
    immutable_value i_get(int i) const;

    /*@}*/


  protected:

    /** The type used to store the subexpression.  The expression is stored
     * as a copy if Sub is an rvalue reference (temporary), or by const
     * reference if Sub is an lvalue reference.
     */
    typedef cml::if_t<std::is_lvalue_reference<Sub>::value,
	    const sub_type&, sub_type>			sub_wrap_type;


  protected:

    /** The subexpression. */
    sub_wrap_type		m_sub;


  private:

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    // Not copy constructible.
    quaternion_unary_node(const node_type&);
#endif

    // Not assignable.
    node_type& operator=(const node_type&);
};

} // namespace cml

#define __CML_QUATERNION_UNARY_NODE_TPP
#include <cml/quaternion/unary_node.tpp>
#undef __CML_QUATERNION_UNARY_NODE_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
