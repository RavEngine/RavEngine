/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_fixed_compiled_h
#define	cml_quaternion_fixed_compiled_h

#include <cml/storage/compiled_selector.h>
#include <cml/quaternion/writable_quaternion.h>
#include <cml/quaternion/quaternion.h>

namespace cml {

template<class Element, class Order, class Cross>
struct quaternion_traits< quaternion<Element, fixed<>, Order, Cross> >
{
  /* Traits and types for the quaternion element: */
  typedef scalar_traits<Element>			element_traits;
  typedef typename element_traits::value_type		value_type;
  typedef typename element_traits::pointer		pointer;
  typedef typename element_traits::reference		reference;
  typedef typename element_traits::const_pointer	const_pointer;
  typedef typename element_traits::const_reference	const_reference;
  typedef typename element_traits::mutable_value	mutable_value;
  typedef typename element_traits::immutable_value	immutable_value;

  /* The quaternion storage type: */
  typedef rebind_t<compiled<4>, quaternion_storage_tag>	storage_type;
  typedef typename storage_type::size_tag		size_tag;
  static_assert(std::is_same<size_tag, fixed_size_tag>::value,
    "invalid size tag");

  /* Array size (should be positive): */
  static const int array_size = storage_type::array_size;
  static_assert(array_size == 4, "invalid quaternion size");

  /** Quaternion order. */
  typedef Order						order_type;

  /** Quaternion cross type. */
  typedef Cross						cross_type;
};

/** Fixed-length quaternion. */
template<class Element, class Order, class Cross>
class quaternion<Element, fixed<>, Order, Cross>
: public writable_quaternion< quaternion<Element, fixed<>, Order, Cross> >
{
  public:

    typedef quaternion<Element, fixed<>, Order, Cross>	quaternion_type;
    typedef readable_quaternion<quaternion_type>	readable_type;
    typedef writable_quaternion<quaternion_type>	writable_type;
    typedef quaternion_traits<quaternion_type>		traits_type;
    typedef typename traits_type::element_traits	element_traits;
    typedef typename traits_type::value_type		value_type;
    typedef typename traits_type::pointer		pointer;
    typedef typename traits_type::reference		reference;
    typedef typename traits_type::const_pointer		const_pointer;
    typedef typename traits_type::const_reference	const_reference;
    typedef typename traits_type::mutable_value		mutable_value;
    typedef typename traits_type::immutable_value	immutable_value;
    typedef typename traits_type::storage_type		storage_type;
    typedef typename traits_type::size_tag		size_tag;
    typedef typename traits_type::order_type		order_type;
    typedef typename traits_type::cross_type		cross_type;


  public:

    /* Include methods from writable_type: */
    using writable_type::W;
    using writable_type::X;
    using writable_type::Y;
    using writable_type::Z;
#ifndef CML_HAS_MSVC_BRAIN_DEAD_ASSIGNMENT_OVERLOADS
    using writable_type::operator=;
#endif


  public:

    /** Constant containing the array size. */
    static const int array_size = traits_type::array_size;

    /** The dimension (same as array_size). */
    static const int dimension = array_size;


  public:

    /** Compiler-default constructor.
     *
     * @note The quaternion elements are uninitialized.
     */
    quaternion() = default;

    /** Compiler-default destructor. */
    ~quaternion() = default;

    /** Compiler-default copy constructor. */
    quaternion(const quaternion_type& other) = default;

#ifdef CML_HAS_DEFAULTED_MOVE_CONSTRUCTOR
    /** Compiler-default move constructor. */
    quaternion(quaternion_type&& other) = default;
#endif

    /** Construct from a readable_quaternion. */
    template<class Sub> quaternion(const readable_quaternion<Sub>& sub);

    /** Construct from 4 values.
     *
     * @note This overload is enabled only if all of the arguments are
     * convertible to value_type.
     */
    template<class E0, class E1, class E2, class E3,
      enable_if_convertible_t<value_type, E0, E1, E2, E3>* = nullptr>
       	quaternion(const E0& e0, const E1& e1, const E2& e2, const E3& e3)
	// XXX Should be in quaternion/fixed_compiled.tpp, but VC++12 has
	// brain-dead out-of-line template argument matching...
	{
	  this->assign_elements(e0, e1, e2, e3);
	}

    /** Construct from a 3D readable_vector and one additional element.
     *
     * @note Although the imaginary part is specified first, the proper
     * coefficient order is maintained.
     *
     * @note This overload is enabled only if the value_type of @c sub and
     * the scalar argument are convertible to value_type.
     */
    template<class Sub, class E0, enable_if_convertible_t<
      value_type, value_type_trait_of_t<Sub>, E0>* = nullptr>
       	quaternion(const readable_vector<Sub>& sub, const E0& e0)
	// XXX Should be in quaternion/fixed_compiled.tpp, but VC++12 has
	// brain-dead out-of-line template argument matching...
	{
	  this->assign(sub, e0);
	}

    /** Construct from one additional element and a 3D readable_vector.
     *
     * @note Although the imaginary part is specified second, the proper
     * coefficient order is maintained.
     *
     * @note This overload is enabled only if the value_type of @c sub and
     * the scalar argument are convertible to value_type.
     */
    template<class E0, class Sub, enable_if_convertible_t<
      value_type, value_type_trait_of_t<Sub>, E0>* = nullptr>
       	quaternion(const E0& e0, const readable_vector<Sub>& sub)
	// XXX Should be in quaternion/fixed_compiled.tpp, but VC++12 has
	// brain-dead out-of-line template argument matching...
	{
	  this->assign(sub, e0);
	}

    /** Construct from a 3-element array and one additional element.
     *
     * @note Although the imaginary part is specified first, the proper
     * coefficient order is maintained.
     */
    template<class Array, class E1, enable_if_array_t<Array>* = nullptr>
      quaternion(const Array& array, const E1& e1);

    /** Construct from one additional element and a 3-element array.
     *
     * @note Although the imaginary part is specified second, the proper
     * coefficient order is maintained.
     */
    template<class E0, class Array, enable_if_array_t<Array>* = nullptr>
      quaternion(const E0& e0, const Array& array);

    /** Construct from an array type. */
    template<class Array, enable_if_array_t<Array>* = nullptr>
      quaternion(const Array& array);

    /** Construct from a pointer to an array. */
    template<class Pointer, enable_if_pointer_t<Pointer>* = nullptr>
      quaternion(const Pointer& array);

    /** Construct from std::initializer_list. */
    template<class Other> quaternion(std::initializer_list<Other> l);


  public:

    /** Return the length of the quaternion. */
    int size() const;

    /** Return access to the quaternion data as a raw pointer. */
    pointer data();

    /** Return const access to the quaternion data as a raw pointer. */
    const_pointer data() const;

    /** Read-only iterator. */
    const_pointer begin() const;

    /** Read-only iterator. */
    const_pointer end() const;


  public:

    /** Copy assignment. */
    quaternion_type& operator=(const quaternion_type& other);

    /** Move assignment. */
    quaternion_type& operator=(quaternion_type&& other);

#ifdef CML_HAS_MSVC_BRAIN_DEAD_ASSIGNMENT_OVERLOADS
    template<class Other>
      quaternion_type& operator=(const readable_quaternion<Other>& other)
      {
	return this->assign(other);
      }

    template<class Array, enable_if_array_t<Array>* = nullptr>
      quaternion_type& operator=(const Array& array)
      {
	return this->assign(array);
      }

    template<class Other>
      quaternion_type& operator=(std::initializer_list<Other> l)
      {
	return this->assign(l);
      }
#endif


  protected:

    /** @name readable_quaternion Interface */
    /*@{*/

    friend readable_type;

    /** Return quaternion const element @c i. */
    immutable_value i_get(int i) const;

    /*@}*/


  protected:

    /** @name writable_quaternion Interface */
    /*@{*/

    friend writable_type;

    /** Return quaternion element @c i. */
    mutable_value i_get(int i);

    /** Set element @c i. */
    template<class Other>
      quaternion_type& i_put(int i, const Other& v) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Set element @c i on a temporary. */
    template<class Other>
      quaternion_type&& i_put(int i, const Other& v) &&;
#endif

    /*@}*/


  protected:

    /** Fixed-length array. */
    value_type			m_data[4];
};

} // namespace cml

#define __CML_QUATERNION_FIXED_COMPILED_TPP
#include <cml/quaternion/fixed_compiled.tpp>
#undef __CML_QUATERNION_FIXED_COMPILED_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
