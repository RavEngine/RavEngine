/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_scalar_node_h
#define	cml_quaternion_scalar_node_h

#include <cml/common/size_tags.h>
#include <cml/quaternion/readable_quaternion.h>

namespace cml {

template<class Sub, class Scalar, class Op> class quaternion_scalar_node;

/** quaternion_scalar_node<> traits. */
template<class Sub, class Scalar, class Op>
struct quaternion_traits< quaternion_scalar_node<Sub,Scalar,Op> >
{
  /* Figure out the basic types of Sub and Scalar: */
  typedef quaternion_scalar_node<Sub,Scalar,Op>		quaternion_type;
  typedef Sub						left_arg_type;
  typedef Scalar 					right_arg_type;
  typedef cml::unqualified_type_t<Sub>			left_type;
  typedef cml::unqualified_type_t<Scalar>		right_type;
  typedef quaternion_traits<left_type>			left_traits;
  typedef scalar_traits<typename Op::result_type>	element_traits;
  typedef typename element_traits::value_type		value_type;
  typedef value_type					immutable_value;
  typedef typename left_traits::storage_type		storage_type;

  typedef typename left_traits::size_tag		size_tag;
  static_assert(cml::is_fixed_size<storage_type>::value, "invalid size tag");

  /* Array size: */
  static const int array_size = storage_type::array_size;
  static_assert(array_size == 4, "invalid quaternion size");

  /* Order and cross taken from the sub-expression: */
  typedef typename left_traits::order_type		order_type;
  typedef typename left_traits::cross_type		cross_type;
};

/** Represents a binary quaternion operation, where one operand is a scalar
 * value, and the other is a quaternion.
 */
template<class Sub, class Scalar, class Op>
class quaternion_scalar_node
: public readable_quaternion< quaternion_scalar_node<Sub,Scalar,Op> >
{
  public:

    typedef quaternion_scalar_node<Sub,Scalar,Op>	node_type;
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

    /** The array size constant is the same as the subexpression. */
    static const int array_size = traits_type::array_size;


  public:

    /** Construct from the wrapped sub-expression and the scalar to apply.
     * @c left must be an lvalue reference or rvalue reference.
     */
    quaternion_scalar_node(Sub left, const right_type& right);

    /** Move constructor. */
    quaternion_scalar_node(node_type&& other);

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Copy constructor. */
    quaternion_scalar_node(const node_type& other);
#endif


  protected:

    /** @name readable_quaternion Interface */
    /*@{*/

    friend readable_type;

    /** Apply the scalar operator to element @c i of the subexpression and
     * return the result.
     */
    immutable_value i_get(int i) const;

    /*@}*/


  protected:

    /** The type used to store the left subexpression.  The expression is
     * stored as a copy if Sub is an rvalue reference (temporary), or by
     * const reference if Sub is an lvalue reference.
     */
    typedef cml::if_t<std::is_lvalue_reference<Sub>::value,
	    const left_type&, left_type>		left_wrap_type;


  protected:

    /** The quaternion operand. */
    left_wrap_type		m_left;

    /** The scalar operand. */
    right_type			m_right;


  private:

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    // Not copy constructible.
    quaternion_scalar_node(const node_type&);
#endif

    // Not assignable.
    node_type& operator=(const node_type&);
};

} // namespace cml

#define __CML_QUATERNION_SCALAR_NODE_TPP
#include <cml/quaternion/scalar_node.tpp>
#undef __CML_QUATERNION_SCALAR_NODE_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
