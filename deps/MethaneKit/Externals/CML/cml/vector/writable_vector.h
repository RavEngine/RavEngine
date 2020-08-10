/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_writable_vector_h
#define	cml_vector_writable_vector_h

#include <initializer_list>
#include <cml/common/mpl/enable_if_t.h>
#include <cml/common/mpl/enable_if_pointer.h>
#include <cml/common/mpl/enable_if_array.h>
#include <cml/common/mpl/enable_if_convertible.h>
#include <cml/vector/readable_vector.h>

namespace cml {

/** Base class for writable vector types.  Writable vectors support
 * non-const read-write access to its elements, in addition to read-only
 * access via readable_vector.
 *
 * In addition to the requirements of readable_vector, DerivedT must
 * implement:
 *
 * - <X> i_get(int i), where <X> is the mutable_value type defined by
 * vector_traits<DerivedT>
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
class writable_vector
: public readable_vector<DerivedT>
{
  public:

    typedef DerivedT					vector_type;
    typedef readable_vector<vector_type>		readable_type;
    typedef vector_traits<vector_type>			traits_type;
    typedef typename traits_type::value_type		value_type;
    typedef typename traits_type::const_reference	const_reference;
    typedef typename traits_type::mutable_value		mutable_value;


  public:

    /* Disambiguate readable_vector<> methods: */
    using readable_type::actual;
    using readable_type::get;
    using readable_type::normalize;
    using readable_type::operator[];


  public:

    /** Return a mutable reference to the vector cast as DerivedT. */
    DerivedT& actual();

    /** Set element @c i. */
    template<class Other> DerivedT& put(int i, const Other& v) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Set element @c i on a temporary. */
    template<class Other> DerivedT&& put(int i, const Other& v) &&;
#endif

    /** Return mutable element @c i. */
    mutable_value get(int i);

#ifdef CML_HAS_STRUCTURED_BINDINGS
    /** Return const element @c i. */
    template<std::size_t I, enable_if_fixed_size<vector_traits<DerivedT>>* = nullptr>
    mutable_value get();
#endif

    /** Return a mutable reference to element @c i. */
    mutable_value operator[](int i);


  public:

    /** Divide the vector elements by the length of the vector. */
    DerivedT& normalize() __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Divide the vector elements of a temporary by the length of the
     * vector.
     */
    DerivedT&& normalize() &&;
#endif

    /** Zero the vector elements. */
    DerivedT& zero() __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Zero the vector elements of a temporary. */
    DerivedT&& zero() &&;
#endif

    /** Set element @c i to value_type(1), and the other elements to 0. */
    DerivedT& cardinal(int i) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Set element @c i of a temporary to value_type(1), and the other
     * elements to 0.
     */
    DerivedT&& cardinal(int i) &&;
#endif

    /** Set the vector to the pairwise minimum elements with @c other.
     *
     * @throws incompatible_vector_size_error at run-time if either vector is
     * dynamically-sized, and @c other.size() != this->size().  If both are
     * fixed-size expressions, then the size is checked at compile time.
     */
    template<class OtherDerivedT> DerivedT&
      minimize(const readable_vector<OtherDerivedT>& other) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Set the temporary vector to the pairwise minimum elements with @c
     * other.
     *
     * @throws incompatible_vector_size_error at run-time if either vector is
     * dynamically-sized, and @c other.size() != this->size().  If both are
     * fixed-size expressions, then the size is checked at compile time.
     */
    template<class OtherDerivedT> DerivedT&&
      minimize(const readable_vector<OtherDerivedT>& other) &&;
#endif

    /** Set the vector to the pairwise minimum elements with @c other.
     *
     * @throws incompatible_vector_size_error at run-time if either vector is
     * dynamically-sized, and @c other.size() != this->size().  If both are
     * fixed-size expressions, then the size is checked at compile time.
     */
    template<class OtherDerivedT> DerivedT&
      maximize(const readable_vector<OtherDerivedT>& other) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Set the vector to the pairwise minimum elements with @c other.
     *
     * @throws incompatible_vector_size_error at run-time if either vector is
     * dynamically-sized, and @c other.size() != this->size().  If both are
     * fixed-size expressions, then the size is checked at compile time.
     */
    template<class OtherDerivedT> DerivedT&&
      maximize(const readable_vector<OtherDerivedT>& other) &&;
#endif

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

    /** Set all elements to a specific value. */
    DerivedT& fill(const_reference v) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Set all elements of a temporary to a specific value. */
    DerivedT&& fill(const_reference v) &&;
#endif


  public:

    /** Assign from a variable list of at least one value. If the vector is
     * resizable, it is resized to exactly accomodate the elements of @c
     * eN.  If the vector is fixed-size, it must have the same length as @c
     * eN.
     *
     * @note This overload is enabled only if all of the arguments are
     * convertible to value_type.
     */
    template<class E0, class... Elements> auto
      set(const E0& e0, const Elements&... eN) __CML_REF -> enable_if_t<
	are_convertible<value_type, E0, Elements...>::value, DerivedT&>;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Assign a temporary from a variable list of at least one value. If
     * the vector is resizable, it is resized to exactly accomodate the
     * elements of @c eN.  If the vector is fixed-size, it must have the
     * same length as @c eN.
     *
     * @note This overload is enabled only if all of the arguments are
     * convertible to value_type.
     */
    template<class E0, class... Elements> auto
      set(const E0& e0, const Elements&... eN) && -> enable_if_t<
	are_convertible<value_type, E0, Elements...>::value, DerivedT&&>;
#endif


    /** Copy assignment. */
    DerivedT& operator=(const writable_vector& other) __CML_REF;

    /** Assign from a readable_vector.
     *
     * @throws incompatible_vector_size_error at run-time if the vector is not
     * resizable, and if @c other.size() != this->size().  If both are
     * fixed-size, then the size is checked at compile time.
     */
    template<class OtherDerivedT> DerivedT&
      operator=(const readable_vector<OtherDerivedT>& other) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Assign a temporary from a readable_vector.
     *
     * @throws incompatible_vector_size_error at run-time if the vector is not
     * resizable, and if @c other.size() != this->size().  If both are
     * fixed-size, then the size is checked at compile time.
     */
    template<class OtherDerivedT> DerivedT&&
      operator=(const readable_vector<OtherDerivedT>& other) &&;
#endif

    /** Assign from a fixed-length array type.
     *
     * @throws incompatible_vector_size_error at run-time if the vector is not
     * resizable, and if @c array_size_of_c<value>::value !=
     * this->size().  If both are fixed-size, then the size is checked at
     * compile time.
     */
    template<class Array, enable_if_array_t<Array>* = nullptr>
	DerivedT& operator=(const Array& array) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Assign a temporary from a fixed-length array type.
     *
     * @throws incompatible_vector_size_error at run-time if the vector is not
     * resizable, and if @c array_size_of_c<value>::value !=
     * this->size().  If both are fixed-size, then the size is checked at
     * compile time.
     */
    template<class Array, enable_if_array_t<Array>* = nullptr>
	DerivedT&& operator=(const Array& array) &&;
#endif

    /** Assign from initializer list.
     *
     * @throws incompatible_vector_size_error if the vector is not resizable,
     * and if @c l.size() != this->size().
     */
    template<class Other>
      DerivedT& operator=(std::initializer_list<Other> l) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Assign a temporary from initializer list.
     *
     * @throws incompatible_vector_size_error if the vector is not resizable,
     * and if @c l.size() != this->size().
     */
    template<class Other>
      DerivedT&& operator=(std::initializer_list<Other> l) &&;
#endif

    /** Modify the vector by addition of another vector.
     *
     * @throws incompatible_vector_size_error at run-time if the vector is
     * dynamically-sized, and if @c other.size() != this->size().  If both
     * are fixed-size expressions, then the size is checked at compile
     * time.
     */
    template<class OtherDerivedT> DerivedT&
      operator+=(const readable_vector<OtherDerivedT>& other) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Modify a temporary vector by addition of another vector.
     *
     * @throws incompatible_vector_size_error at run-time if the vector is
     * dynamically-sized, and if @c other.size() != this->size().  If both
     * are fixed-size expressions, then the size is checked at compile
     * time.
     */
    template<class OtherDerivedT> DerivedT&&
      operator+=(const readable_vector<OtherDerivedT>& other) &&;
#endif

    /** Modify the vector by subtraction of another vector.
     *
     * @throws incompatible_vector_size_error at run-time if the vector is
     * dynamically-sized, and if @c other.size() != this->size().  If both
     * are fixed-size expressions, then the size is checked at compile
     * time.
     */
    template<class OtherDerivedT> DerivedT&
      operator-=(const readable_vector<OtherDerivedT>& other) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Modify a temporary vector by subtraction of another vector.
     *
     * @throws incompatible_vector_size_error at run-time if the vector is
     * dynamically-sized, and if @c other.size() != this->size().  If both
     * are fixed-size expressions, then the size is checked at compile
     * time.
     */
    template<class OtherDerivedT> DerivedT&&
      operator-=(const readable_vector<OtherDerivedT>& other) &&;
#endif

    /** Multiply the vector by a scalar convertible to its value_type. */
    template<class ScalarT,
      enable_if_convertible_t<value_type, ScalarT>* = nullptr>
	DerivedT& operator*=(const ScalarT& v) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Multiply the temporary vector by a scalar convertible to its
     * value_type.
     */
    template<class ScalarT,
      enable_if_convertible_t<value_type, ScalarT>* = nullptr>
	DerivedT&& operator*=(const ScalarT& v) &&;
#endif

    /** Divide the vector by a scalar convertible to its value_type. */
    template<class ScalarT,
      enable_if_convertible_t<value_type, ScalarT>* = nullptr>
	DerivedT& operator/=(const ScalarT& v) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Divide the vector temporary by a scalar convertible to its
     * value_type.
     */
    template<class ScalarT,
      enable_if_convertible_t<value_type, ScalarT>* = nullptr>
	DerivedT&& operator/=(const ScalarT& v) &&;
#endif


  protected:

    /** Assign from a readable_vector.
     *
     * @note This depends upon implicit conversion of the source vector
     * elements to the vector value_type.
     *
     * @throws incompatible_vector_size_error at run-time if the vector is not
     * resizable, and if @c other.size() != this->size().  If both are
     * fixed-size expressions, then the size is checked at compile time.
     */
    template<class OtherDerivedT>
      DerivedT& assign(const readable_vector<OtherDerivedT>& other);

    /** Construct from a fixed-length array of values.  If the vector is
     * resizable, it is resized to exactly accomodate the array.  If the
     * vector is fixed-size, it must have the same length as @c array.
     *
     * @note This depends upon implicit conversions of the elements to the
     * vector value_type.
     */
    template<class Array, cml::enable_if_array_t<Array>* = nullptr>
      DerivedT& assign(const Array& array);

    /** Assign from a pointer to an array.
     *
     * @note This depends upon implicit conversion of the array elements to
     * the vector value_type.
     *
     * @note The number of elements read from @c array depends upon the
     * current size of the vector.
     */
    template<class Pointer, cml::enable_if_pointer_t<Pointer>* = nullptr>
      DerivedT& assign(const Pointer& array);

    /** Construct from an initializer_list. If the vector is resizable, it
     * is resized to exactly accomodate the elements of @c l. If the vector
     * is fixed-size, it must have the same length as @c array.
     *
     * @note This depends upon implicit conversions of the elements to the
     * vector value_type.
     */
    template<class Other>
      DerivedT& assign(const std::initializer_list<Other>& l);

    /** Assign from a subvector and 1 or more additional elements to
     * append.
     */
    template<class OtherDerivedT, class... Elements>
      DerivedT& assign(
	const readable_vector<OtherDerivedT>& other, const Elements&... eN);

    /** Construct from a variable list of values. If the vector is
     * resizable, it is resized to exactly accomodate the elements of @c
     * eN.  If the vector is fixed-size, it must have the same length as @c
     * eN.
     *
     * @note This depends upon implicit conversions of the elements to the
     * vector value_type.
     */
    template<class... Elements>
      DerivedT& assign_elements(const Elements&... eN);


  protected:

    // Use the compiler-generated default constructor:
    writable_vector() = default;

    // Use the compiler-generated copy constructor:
    writable_vector(const writable_vector&) = default;

#ifdef CML_HAS_DEFAULTED_MOVE_CONSTRUCTOR
    // Use the compiler-generated move constructor:
    writable_vector(writable_vector&&) = default;
#endif
};

} // namespace cml

#define __CML_VECTOR_WRITABLE_VECTOR_TPP
#include <cml/vector/writable_vector.tpp>
#undef __CML_VECTOR_WRITABLE_VECTOR_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
