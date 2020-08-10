/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_readable_quaternion_h
#define	cml_quaternion_readable_quaternion_h

#include <cml/common/compiler.h>
#include <cml/scalar/binary_ops.h>
#include <cml/quaternion/traits.h>

namespace cml {

/* Forward declarations: */
template<class Sub, class Scalar, class Op> class quaternion_scalar_node;
template<class Sub> class imaginary_node;
template<class Sub> class conjugate_node;
template<class Sub> class inverse_node;

/** Base class for readable quaternion types.  Readable quaternions support
 * const access to its elements.
 *
 * DerivedT must implement:
 *
 * - <X> i_get(int i) const, where <X> is the immutable_value type defined
 * by quaternion_traits<DerivedT>.  Note that immutable_value is not
 * necessarily a reference or const type.
 */
template<class DerivedT>
class readable_quaternion
{
  public:

    typedef DerivedT					quaternion_type;
    typedef quaternion_traits<quaternion_type>		traits_type;
    typedef typename traits_type::element_traits	element_traits;
    typedef typename traits_type::value_type		value_type;
    typedef typename traits_type::immutable_value	immutable_value;
    typedef typename traits_type::order_type		order_type;
    typedef typename traits_type::cross_type		cross_type;


  public:

    /** Localize the ordering as an enum. */
    enum {
        W = order_type::W,
        X = order_type::X,
        Y = order_type::Y,
        Z = order_type::Z
    };


  public:

    /** Return a const reference to the quaternion cast as DerivedT. */
    const DerivedT& actual() const;

    /** Return const element @c i. */
    immutable_value get(int i) const;

    /** Return const element @c i. */
    immutable_value operator[](int i) const;

    /** Return a const reference to the real part of the quaternion. */
    immutable_value w() const;

    /** Return a const reference to the imaginary i coordinate */
    immutable_value x() const;

    /** Return a const reference to the imaginary j coordinate */
    immutable_value y() const;

    /** Return a const reference to the imaginary k coordinate */
    immutable_value z() const;


  public:

    /** Return the array size.  This is always 4. */
    int size() const;

    /** Return the real part of the quaternion. */
    immutable_value real() const;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Return the imaginary part of the quaternion as a vector expression.
     */
    imaginary_node<const DerivedT&> imaginary() const &;

    /** Return the imaginary part of the quaternion as a vector expression,
     * moving the source into the node.
     */
    imaginary_node<DerivedT&&> imaginary() const &&;
#else
    /** Return the imaginary part of the quaternion as a vector expression. */
    imaginary_node<DerivedT> imaginary() const;
#endif

    /** Return the squared length of the quaternion. */
    value_type length_squared() const;

    /** Return the length of the quaternion. */
    value_type length() const;

    /** Return the Cayley norm of the quaternion. */
    value_type norm() const;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Return the normalized quaternion as an expression node. */
    quaternion_scalar_node<const DerivedT&, value_type,
      op::binary_divide<value_type,value_type>> normalize() const &;

    /** Return the normalized quaternion as an expression node, moving the
     * source into the node.
     */
    quaternion_scalar_node<DerivedT&&, value_type,
      op::binary_divide<value_type,value_type>> normalize() const &&;
#else
    /** Return the normalized quaternion as an expression node. */
    quaternion_scalar_node<DerivedT, value_type,
      op::binary_divide<value_type,value_type>> normalize() const;
#endif

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Return the conjugate as an expression node. */
    conjugate_node<const DerivedT&> conjugate() const &;

    /** Return the conjugate as an expression node, moving the source into
     * the node.
     */
    conjugate_node<DerivedT&&> conjugate() const &&;
#else
    /** Return the conjugate as an expression node. */
    conjugate_node<DerivedT> conjugate() const;
#endif

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Return the inverse as an expression node. */
    inverse_node<const DerivedT&> inverse() const &;

    /** Return the inverse as an expression node, moving the source into
     * the node.
     */
    inverse_node<DerivedT&&> inverse() const &&;
#else
    /** Return the inverse as an expression node. */
    inverse_node<DerivedT> inverse() const;
#endif


  protected:

    // Use the compiler-generated default constructor:
    readable_quaternion() = default;

    // Use the compiler-generated copy constructor:
    readable_quaternion(const readable_quaternion&) = default;

#ifdef CML_HAS_DEFAULTED_MOVE_CONSTRUCTOR
    // Use the compiler-generated move constructor:
    readable_quaternion(readable_quaternion&&) = default;
#endif
};

} // namespace cml

#define __CML_QUATERNION_READABLE_QUATERNION_TPP
#include <cml/quaternion/readable_quaternion.tpp>
#undef __CML_QUATERNION_READABLE_QUATERNION_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
