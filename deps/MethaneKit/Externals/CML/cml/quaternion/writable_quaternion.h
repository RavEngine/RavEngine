/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_writable_quaternion_h
#define	cml_quaternion_writable_quaternion_h

#include <initializer_list>
#include <cml/common/mpl/enable_if_t.h>
#include <cml/common/mpl/enable_if_pointer.h>
#include <cml/common/mpl/enable_if_array.h>
#include <cml/common/mpl/enable_if_convertible.h>
#include <cml/vector/fwd.h>
#include <cml/quaternion/readable_quaternion.h>

namespace cml {

/** Base class for writable quaternion types.  Writable quaternions support
 * non-const read-write access to its elements, in addition to read-only
 * access via readable_quaternion.
 *
 * In addition to the requirements of readable_quaternion, DerivedT must
 * implement:
 *
 * - <X> i_get(int i), where <X> is the mutable_value type defined by
 * quaternion_traits<DerivedT>
 *
 * - template<class T> DerivedT& i_put(int i, const T&)
 *
 *   for compilers without support for rvalue reference from *this; and
 *
 *   template<class T> DerivedT& i_put(int i, const T&) &
 *   template<class T> DerivedT&& i_put(int i, const T&) &&
 *
 *   for compilers with support for rvalue reference from this.
 *
 * Note that mutable_value need not be a reference type.
 */
template<class DerivedT>
class writable_quaternion
: public readable_quaternion<DerivedT>
{
  public:

    typedef DerivedT					quaternion_type;
    typedef readable_quaternion<quaternion_type>	readable_type;
    typedef quaternion_traits<quaternion_type>		traits_type;
    typedef typename traits_type::value_type		value_type;
    typedef typename traits_type::const_reference	const_reference;
    typedef typename traits_type::mutable_value		mutable_value;
    typedef typename traits_type::order_type		order_type;


  public:

    /* Disambiguate readable_quaternion<> methods: */
    using readable_type::W;
    using readable_type::X;
    using readable_type::Y;
    using readable_type::Z;
    using readable_type::actual;
    using readable_type::get;
    using readable_type::normalize;
    using readable_type::conjugate;
    using readable_type::inverse;
    using readable_type::operator[];
    using readable_type::x;
    using readable_type::y;
    using readable_type::z;
    using readable_type::w;


  public:

    /** Return a mutable reference to the quaternion cast as DerivedT. */
    DerivedT& actual();

    /** Set element @c i. */
    template<class Other> DerivedT& put(int i, const Other& v) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Set element @c i on a temporary. */
    template<class Other> DerivedT&& put(int i, const Other& v) &&;
#endif

    /** Return mutable element @c i. */
    mutable_value get(int i);

    /** Return a mutable reference to element @c i. */
    mutable_value operator[](int i);

    /** Return a mutable reference to the real part of the quaternion. */
    mutable_value w();

    /** Return a mutable reference to the imaginary i coordinate. */
    mutable_value x();

    /** Return a mutable reference to the imaginary j coordinate. */
    mutable_value y();

    /** Return a mutable reference to the imaginary k coordinate. */
    mutable_value z();


  public:

    /** Set the scalar of the quaternion to @c s, and the imaginary
     * vector to @c v.
     *
     * @note This functin is enabled only if the value_type of @c v and @c
     * E are convertible to value_type.
     */
    template<class Sub, class E, enable_if_vector_t<Sub>* = nullptr> auto
	set(const readable_vector<Sub>& v, const E& s) __CML_REF
	-> enable_if_t<are_convertible<
	  value_type, value_type_trait_of_t<Sub>, E>::value, DerivedT&>;

    /** Set the scalar of the quaternion to @c s, and the imaginary
     * vector to @c v.
     *
     * @note This functin is enabled only if the value_type of @c v and @c
     * E are convertible to value_type.
     */
    template<class E, class Sub, enable_if_vector_t<Sub>* = nullptr> auto
	set(const E& s, const readable_vector<Sub>& v) __CML_REF
	-> enable_if_t<are_convertible<
	  value_type, value_type_trait_of_t<Sub>, E>::value, DerivedT&>;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Set the scalar of the temporary quaternion to @c s, and the
     * imaginary vector to @c v.
     *
     * @note This functin is enabled only if the value_type of @c v and @c
     * E are convertible to value_type.
     */
    template<class Sub, class E, enable_if_vector_t<Sub>* = nullptr> auto
	set(const readable_vector<Sub>& v, const E& s) &&
	-> enable_if_t<are_convertible<
	  value_type, value_type_trait_of_t<Sub>, E>::value, DerivedT&&>;

    /** Set the scalar of the temporary quaternion to @c s, and the
     * imaginary vector to @c v.
     *
     * @note This functin is enabled only if the value_type of @c v and @c
     * E are convertible to value_type.
     */
    template<class E, class Sub, enable_if_vector_t<Sub>* = nullptr> auto
	set(const E& s, const readable_vector<Sub>& v) &&
	-> enable_if_t<are_convertible<
	  value_type, value_type_trait_of_t<Sub>, E>::value, DerivedT&&>;
#endif


  public:

    /** Divide the quaternion elements by the length of the quaternion. */
    DerivedT& normalize() __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Divide the quaternion elements of a temporary by the length of the
     * quaternion.
     */
    DerivedT&& normalize() &&;
#endif

    /** Zero the quaternion elements. */
    DerivedT& zero() __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Zero the quaternion elements of a temporary. */
    DerivedT&& zero() &&;
#endif

    /** Set the quaternion to the identity. */
    DerivedT& identity() __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Set a temporary to the identity. */
    DerivedT&& identity() &&;
#endif

    /** Set the quaternion to its conjugate. */
    DerivedT& conjugate() __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Set a temporary to its conjugate. */
    DerivedT&& conjugate() &&;
#endif

    /** Set the quaternion to its inverse. */
    DerivedT& inverse() __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Set a temporary to its inverse. */
    DerivedT&& inverse() &&;
#endif

    /** Set the quaternion to its natural logarithm.
     *
     * @note It is up to the caller to ensure the quaternion has a usable
     * non-zero length.
     *
     * @breaking In CML1, this function returns a temporary. Use cml::log()
     * as a replacement.
     */
    DerivedT& log() __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Set a temporary to its natural logarithm.
     *
     * @note It is up to the caller to ensure the quaternion has a usable
     * non-zero length.
     *
     * @breaking In CML1, this function returns a temporary. Use cml::log()
     * as a replacement.
     */
    DerivedT&& log() &&;
#endif

    /** Set the quaternion to its exponential.
     *
     * @breaking In CML1, this function returns a temporary. Use cml::exp()
     * as a replacement.
     */
    DerivedT& exp() __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Set a temporary to its exponential.
     *
     * @breaking In CML1, this function returns a temporary. Use cml::exp()
     * as a replacement.
     */
    DerivedT&& exp() &&;
#endif


#if 0
    /** Set elements to random values in the range @c[low,high].
     *
     * @note Use std::srand() to seed the random number generator.
     */
    DerivedT& random(const_reference low, const_reference high) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Set elements of a temporary to random values in the range
     * @c[low,high].
     */
    DerivedT&& random(const_reference low, const_reference high) &&;
#endif
#endif


  public:

    /** Assign from a readable_quaternion. */
    template<class OtherDerivedT> DerivedT&
      operator=(const readable_quaternion<OtherDerivedT>& other) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Assign a temporary from a readable_quaternion. */
    template<class OtherDerivedT> DerivedT&&
      operator=(const readable_quaternion<OtherDerivedT>& other) &&;
#endif

    /** Assign from a fixed-length array type.
     *
     * @throws incompatible_quaternion_size_error if @c
     * array_size_of_c<value>::value != 4.
     */
    template<class Array, enable_if_array_t<Array>* = nullptr>
	DerivedT& operator=(const Array& array) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Assign a temporary from a fixed-length array type.
     *
     * @throws incompatible_quaternion_size_error if @c
     * array_size_of_c<value>::value != 4.
     */
    template<class Array, enable_if_array_t<Array>* = nullptr>
	DerivedT&& operator=(const Array& array) &&;
#endif

    /** Assign from initializer list.
     *
     * @throws incompatible_quaternion_size_error if if @c l.size() != 4.
     */
    template<class Other>
      DerivedT& operator=(std::initializer_list<Other> l) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Assign a temporary from initializer list.
     *
     * @throws incompatible_quaternion_size_error if if @c l.size() != 4.
     */
    template<class Other>
      DerivedT&& operator=(std::initializer_list<Other> l) &&;
#endif

    /** Modify the quaternion by addition of another quaternion. */
    template<class OtherDerivedT> DerivedT&
      operator+=(const readable_quaternion<OtherDerivedT>& other) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Modify a temporary quaternion by addition of another quaternion. */
    template<class OtherDerivedT> DerivedT&&
      operator+=(const readable_quaternion<OtherDerivedT>& other) &&;
#endif

    /** Modify the quaternion by subtraction of another quaternion. */
    template<class OtherDerivedT> DerivedT&
      operator-=(const readable_quaternion<OtherDerivedT>& other) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Modify a temporary quaternion by subtraction of another quaternion. */
    template<class OtherDerivedT> DerivedT&&
      operator-=(const readable_quaternion<OtherDerivedT>& other) &&;
#endif

    /** Modify the quaternion by multiplication of another quaternion. */
    template<class OtherDerivedT> DerivedT&
      operator*=(const readable_quaternion<OtherDerivedT>& other) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Modify a temporary quaternion by multiplication of another
     * quaternion.
     */
    template<class OtherDerivedT> DerivedT&&
      operator*=(const readable_quaternion<OtherDerivedT>& other) &&;
#endif

    /** Multiply the quaternion by a scalar convertible to its value_type. */
    template<class ScalarT,
      typename enable_if_convertible<value_type, ScalarT>::type* = nullptr>
	DerivedT& operator*=(const ScalarT& v) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Multiply the temporary quaternion by a scalar convertible to its
     * value_type.
     */
    template<class ScalarT,
      typename enable_if_convertible<value_type, ScalarT>::type* = nullptr>
	DerivedT&& operator*=(const ScalarT& v) &&;
#endif

    /** Divide the quaternion by a scalar convertible to its value_type. */
    template<class ScalarT,
      typename enable_if_convertible<value_type, ScalarT>::type* = nullptr>
	DerivedT& operator/=(const ScalarT& v) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Divide the quaternion temporary by a scalar convertible to its
     * value_type.
     */
    template<class ScalarT,
      typename enable_if_convertible<value_type, ScalarT>::type* = nullptr>
	DerivedT&& operator/=(const ScalarT& v) &&;
#endif


  protected:

    /** Assign from a readable_quaternion.
     *
     * @note This depends upon implicit conversion of the source quaternion
     * elements to the quaternion value_type.
     */
    template<class OtherDerivedT>
      DerivedT& assign(const readable_quaternion<OtherDerivedT>& other);

    /** Assign from a readable_vector and a scalar. */
    template<class OtherDerivedT, class E0> DerivedT&
      assign(const readable_vector<OtherDerivedT>& other, const E0& e0);

    /** Construct from a fixed-length array of values.  The assignment
     * order is determined by the quaternion order.
     *
     * @note This depends upon implicit conversions of the elements to the
     * quaternion value_type.
     */
    template<class Array, enable_if_array_t<Array>* = nullptr>
      DerivedT& assign(const Array& array);

    /** Assign from a pointer to an array.
     *
     * @note This depends upon implicit conversion of the array elements to
     * the quaternion value_type.
     */
    template<class Pointer, enable_if_pointer_t<Pointer>* = nullptr>
      DerivedT& assign(const Pointer& array);

    /** Construct from an array of 3 values and one additional element.
     * The assignment order is determined by the quaternion order.
     *
     * @note This depends upon implicit conversions of the elements to the
     * quaternion value_type.
     */
    template<class Array, class E0, enable_if_array_t<Array>* = nullptr>
      DerivedT& assign(const Array& array, const E0& e0);

    /** Construct from an initializer_list.
     *
     * @note This depends upon implicit conversions of the elements to the
     * quaternion value_type.
     */
    template<class Other>
      DerivedT& assign(const std::initializer_list<Other>& l);

    /** Construct from a list of 4 values.
     *
     * @note This depends upon implicit conversions of the elements to the
     * quaternion value_type.
     */
    template<class E0, class E1, class E2, class E3> DerivedT&
      assign_elements(const E0& e0, const E1& e1, const E2& e2, const E3& e3);


  protected:

    // Use the compiler-generated default constructor:
    writable_quaternion() = default;

    // Use the compiler-generated copy constructor:
    writable_quaternion(const writable_quaternion&) = default;

#ifdef CML_HAS_DEFAULTED_MOVE_CONSTRUCTOR
    // Use the compiler-generated move constructor:
    writable_quaternion(writable_quaternion&&) = default;
#endif

    // Force assignment through operator=(readable_quaternion<>):
    writable_quaternion& operator=(const writable_quaternion&) = delete;
};

} // namespace cml

#define __CML_QUATERNION_WRITABLE_QUATERNION_TPP
#include <cml/quaternion/writable_quaternion.tpp>
#undef __CML_QUATERNION_WRITABLE_QUATERNION_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
