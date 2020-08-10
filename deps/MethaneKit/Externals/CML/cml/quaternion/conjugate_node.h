/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_conjugate_node_h
#define	cml_quaternion_conjugate_node_h

#include <cml/quaternion/readable_quaternion.h>

namespace cml {

template<class Sub> class conjugate_node;

/** conjugate_node<> traits. */
template<class Sub>
struct quaternion_traits< conjugate_node<Sub> >
{
  /* Figure out the basic type of Sub: */
  typedef conjugate_node<Sub>				quaternion_type;
  typedef Sub						sub_arg_type;
  typedef cml::unqualified_type_t<Sub>			sub_type;
  typedef quaternion_traits<sub_type>			sub_traits;
  typedef scalar_traits<value_type_of_t<sub_traits>>	element_traits;
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

/** Represents the conjugate of a quaternion subexpression. */
template<class Sub>
class conjugate_node
: public readable_quaternion< conjugate_node<Sub> >
{
  public:

    typedef conjugate_node<Sub>				node_type;
    typedef readable_quaternion<node_type>		readable_type;
    typedef quaternion_traits<node_type>		traits_type;
    typedef typename traits_type::sub_arg_type		sub_arg_type;
    typedef typename traits_type::sub_type		sub_type;
    typedef typename traits_type::element_traits	element_traits;
    typedef typename traits_type::value_type		value_type;
    typedef typename traits_type::immutable_value	immutable_value;
    typedef typename traits_type::storage_type		storage_type;
    typedef typename traits_type::size_tag		size_tag;


  public:

    /** The array size constant depends upon the subexpression size. */
    static const int array_size = traits_type::array_size;


  public:

    /** Construct from the wrapped quaternion expression.  @c sub must be
     * an lvalue reference or rvalue reference.
     */
    explicit conjugate_node(Sub sub);

    /** Move constructor. */
    conjugate_node(node_type&& other);

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Copy constructor. */
    conjugate_node(const node_type& other);
#endif


  protected:

    /** @name readable_quaternion Interface */
    /*@{*/

    friend readable_type;

    /** Apply the operator to element @c i and return the result. */
    immutable_value i_get(int i) const;

    /*@}*/


  protected:

    /** The type used to store the subexpression.  The expression is stored
     * as a copy if Sub is an rvalue reference (temporary), or by const
     * reference if Sub is an lvalue reference.
     */
    typedef cml::if_t<std::is_lvalue_reference<Sub>::value,
	    const sub_type&, sub_type>			wrap_type;

    /** The wrapped subexpression. */
    wrap_type			m_sub;


  private:

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    // Not copy constructible.
    conjugate_node(const node_type&);
#endif

    // Not assignable.
    node_type& operator=(const node_type&);
};

} // namespace cml

#define __CML_QUATERNION_CONJUGATE_NODE_TPP
#include <cml/quaternion/conjugate_node.tpp>
#undef __CML_QUATERNION_CONJUGATE_NODE_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
