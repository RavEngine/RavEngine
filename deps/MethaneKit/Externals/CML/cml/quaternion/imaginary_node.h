/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_imaginary_node_h
#define	cml_quaternion_imaginary_node_h

#include <cml/vector/readable_vector.h>
#include <cml/quaternion/readable_quaternion.h>

namespace cml {

template<class Sub> class imaginary_node;

/** imaginary_node<> traits. */
template<class Sub>
struct vector_traits< imaginary_node<Sub> >
{
  /* Figure out the basic type of Sub: */
  typedef imaginary_node<Sub>				vector_type;
  typedef Sub						sub_arg_type;
  typedef cml::unqualified_type_t<Sub>			sub_type;
  typedef quaternion_traits<sub_type>			sub_traits;
  typedef typename sub_traits::element_traits		element_traits;
  typedef typename sub_traits::value_type		value_type;
  typedef typename sub_traits::immutable_value		immutable_value;

  /* Resize the *unbound* storage type of the quaternion subexpression to a
   * vector storage type:
   */
  typedef typename sub_traits::storage_type		bound_storage_type;
  typedef typename bound_storage_type::unbound_type	unbound_storage_type;
  typedef resize_storage_t<unbound_storage_type, 3>	resized_type;

  /* Rebind to vector storage: */
  typedef rebind_t<resized_type, vector_storage_tag>	storage_type;

  /* Traits and types for the new storage: */
  typedef typename storage_type::size_tag		size_tag;
  static_assert(cml::is_fixed_size<storage_type>::value, "invalid size tag");

  /* Array size: */
  static const int array_size = storage_type::array_size;
  static_assert(array_size == 3, "invalid imaginary vector size");
};

/** Represents the imaginary part of a quaternion subexpression as a
 * 3-element vector expression.
 */
template<class Sub>
class imaginary_node
: public readable_vector< imaginary_node<Sub> >
{
  public:

    typedef imaginary_node<Sub>				node_type;
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

    /** The array size constant depends upon the subexpression size. */
    static const int array_size = traits_type::array_size;


  public:

    /** Construct from the wrapped quaternion expression.  @c sub must be
     * an lvalue reference or rvalue reference.
     */
    explicit imaginary_node(Sub sub);

    /** Move constructor. */
    imaginary_node(node_type&& other);

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Copy constructor. */
    imaginary_node(const node_type& other);
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
	    const sub_type&, sub_type>			wrap_type;

    /** The wrapped subexpression. */
    wrap_type			m_sub;


  private:

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    // Not copy constructible.
    imaginary_node(const node_type&);
#endif

    // Not assignable.
    node_type& operator=(const node_type&);
};

} // namespace cml

#define __CML_QUATERNION_IMAGINARY_NODE_TPP
#include <cml/quaternion/imaginary_node.tpp>
#undef __CML_QUATERNION_IMAGINARY_NODE_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
