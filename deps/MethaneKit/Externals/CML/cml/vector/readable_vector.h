/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_readable_vector_h
#define	cml_vector_readable_vector_h

#include <cml/common/compiler.h>
#include <cml/scalar/binary_ops.h>
#include <cml/vector/temporary.h>

namespace cml {

/* Forward declarations: */
template<class Sub, class Scalar, class Op> class vector_scalar_node;
template<class Sub> class subvector_node;

/** Base class for readable vector types.  Readable vectors support const
 * access to its elements.
 *
 * DerivedT must implement:
 *
 * - int i_size() const, returning the number of vector elements (even if
 * static); and
 *
 * - <X> i_get(int i) const, where <X> is the immutable_value type defined
 * by vector_traits<DerivedT>.  Note that immutable_value is not
 * necessarily a reference or const type.
 */
template<class DerivedT>
class readable_vector
{
  public:

    typedef DerivedT					vector_type;
    typedef vector_traits<vector_type>			traits_type;
    typedef typename traits_type::element_traits	element_traits;
    typedef typename traits_type::value_type		value_type;
    typedef typename traits_type::immutable_value	immutable_value;
    typedef typename traits_type::size_tag	        size_tag;


  public:

    /** @name CML1 Types */
    /*@{*/

    typedef subvector_of_t<DerivedT>			subvector_type;
    typedef supervector_of_t<DerivedT>			supervector_type;

    /*@}*/


  public:

    /** Return a const reference to the vector cast as DerivedT. */
    const DerivedT& actual() const;

    /** Return the number of vector elements. */
    int size() const;

    /** Return const element @c i. */
    immutable_value get(int i) const;

#ifdef CML_HAS_STRUCTURED_BINDINGS
    /** Return const element @c i. */
    template<std::size_t I, enable_if_fixed_size<vector_traits<DerivedT>>* = nullptr>
    immutable_value get() const;
#endif

    /** Return const element @c i. */
    immutable_value operator[](int i) const;


  public:

    /** Return the squared length of the vector. */
    value_type length_squared() const;

    /** Return the length of the vector. */
    value_type length() const;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Return the normalized vector as an expression node. */
    vector_scalar_node<const DerivedT&, value_type,
      op::binary_divide<value_type,value_type>> normalize() const &;

    /** Return the normalized vector as an expression node, moving the
     * source into the node.
     */
    vector_scalar_node<DerivedT&&, value_type,
      op::binary_divide<value_type,value_type>> normalize() const &&;
#else
    /** Return the normalized vector as an expression node. */
    vector_scalar_node<DerivedT, value_type,
      op::binary_divide<value_type,value_type>> normalize() const;
#endif

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Return subvector @c i as an expression node. */
    subvector_node<const DerivedT&> subvector(int i) const &;

    /** Return subvector @c i as an expression node, moving the source
     * into the node.
     */
    subvector_node<DerivedT&&> subvector(int i) const &&;
#else
    /** Return subvector @c i as an expression node. */
    subvector_node<DerivedT> subvector(int i) const;
#endif


  protected:

    // Use the compiler-generated default constructor:
    readable_vector() = default;

    // Use the compiler-generated copy constructor:
    readable_vector(const readable_vector&) = default;

#ifdef CML_HAS_DEFAULTED_MOVE_CONSTRUCTOR
    // Use the compiler-generated move constructor:
    readable_vector(readable_vector&&) = default;
#endif
};

} // namespace cml

#define __CML_VECTOR_READABLE_VECTOR_TPP
#include <cml/vector/readable_vector.tpp>
#undef __CML_VECTOR_READABLE_VECTOR_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
