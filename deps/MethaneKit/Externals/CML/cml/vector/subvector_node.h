/* -*- C++ -*- ------------------------------------------------------------ @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_subvector_node_h
#define	cml_vector_subvector_node_h

#include <cml/vector/readable_vector.h>
#include <cml/vector/promotion.h>

namespace cml {

template<class Sub> class subvector_node;

/** subvector_node<> traits. */
template<class Sub>
struct vector_traits< subvector_node<Sub> >
{
  /* Figure out the basic type of Sub: */
  typedef subvector_node<Sub>				vector_type;
  typedef Sub						sub_arg_type;
  typedef cml::unqualified_type_t<Sub>			sub_type;
  typedef vector_traits<sub_type>			sub_traits;
  typedef typename sub_traits::element_traits		element_traits;
  typedef typename sub_traits::value_type		value_type;
  typedef typename sub_traits::immutable_value		immutable_value;

  /* Compute the new storage size: */
  private:
  static const int old_array_size = sub_traits::array_size;
  static const int new_array_size = old_array_size - 1;
  static const int N = new_array_size > 0 ? new_array_size : -1;
  public:

  /* Resize the storage type of the subexpression: */
  typedef resize_storage_t<
    storage_type_of_t<sub_traits>, N>			resized_type;

  /* Rebind to vector storage: */
  typedef rebind_t<resized_type, vector_storage_tag>	storage_type;

  /* Traits and types for the new storage: */
  typedef typename storage_type::size_tag		size_tag;

  /* Array size: */
  static const int array_size = storage_type::array_size;
};

/** Represents an N-1 subvector operation in an expression tree, where N is
 * the length of the wrapped subexpression.
 */
template<class Sub>
class subvector_node
: public readable_vector< subvector_node<Sub> >
{
  public:

    typedef subvector_node<Sub>				node_type;
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

    /** Construct from the wrapped sub-expression and the element to drop.
     * @c sub must be an lvalue reference or rvalue reference.
     */
    subvector_node(Sub sub, int skip);

    /** Move constructor. */
    subvector_node(node_type&& other);

#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Copy constructor. */
    subvector_node(const node_type& other);
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

    /** The element to skip. */
    int				m_skip;


  private:

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    // Not copy constructible.
    subvector_node(const node_type&);
#endif

    // Not assignable.
    node_type& operator=(const node_type&);
};

} // namespace cml

#define __CML_VECTOR_SUBVECTOR_NODE_TPP
#include <cml/vector/subvector_node.tpp>
#undef __CML_VECTOR_SUBVECTOR_NODE_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
