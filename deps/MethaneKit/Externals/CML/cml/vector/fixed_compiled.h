/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_fixed_compiled_h
#define	cml_vector_fixed_compiled_h

#include <cml/storage/compiled_selector.h>
#include <cml/vector/writable_vector.h>
#include <cml/vector/vector.h>

namespace cml {

template<class Element, int Size>
struct vector_traits< vector<Element, fixed<Size>> >
{
  /* Traits and types for the vector element: */
  typedef scalar_traits<Element>			element_traits;
  typedef typename element_traits::value_type		value_type;
  typedef typename element_traits::pointer		pointer;
  typedef typename element_traits::reference		reference;
  typedef typename element_traits::const_pointer	const_pointer;
  typedef typename element_traits::const_reference	const_reference;
  typedef typename element_traits::mutable_value	mutable_value;
  typedef typename element_traits::immutable_value	immutable_value;

  /* The vector storage type: */
  typedef rebind_t<compiled<Size>, vector_storage_tag>	storage_type;
  typedef typename storage_type::size_tag		size_tag;
  static_assert(std::is_same<size_tag, fixed_size_tag>::value,
    "invalid size tag");

  /* Array size (should be positive): */
  static const int array_size = storage_type::array_size;
  static_assert(array_size > 0, "invalid vector size");
};

/** Fixed-length vector. */
template<class Element, int Size>
class vector<Element, fixed<Size>>
: public writable_vector< vector<Element, fixed<Size>> >
{
  public:

    typedef vector<Element, fixed<Size>>		vector_type;
    typedef readable_vector<vector_type>		readable_type;
    typedef writable_vector<vector_type>		writable_type;
    typedef vector_traits<vector_type>			traits_type;
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


  public:

    /* Include methods from writable_type: */
    using writable_type::operator[];
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
     * @note The vector elements are uninitialized.
     */
    vector() = default;

    /** Compiler-default destructor. */
    ~vector() = default;

    /** Compiler-default copy constructor. */
    vector(const vector_type& other) = default;

#ifdef CML_HAS_DEFAULTED_MOVE_CONSTRUCTOR
    /** Compiler-default move constructor. */
    vector(vector_type&& other) = default;
#endif

    /** Construct from a readable_vector. */
    template<class Sub> vector(const readable_vector<Sub>& sub);

    /** Construct from at least 1 value.
     *
     * @note This overload is enabled only if all of the arguments are
     * convertible to value_type.
     */
    template<class E0, class... Elements,
      enable_if_convertible_t<value_type, E0, Elements...>* = nullptr>
	vector(const E0& e0, const Elements&... eN)
	// XXX Should be in vector/fixed_compiled.tpp, but VC++12 has
	// brain-dead out-of-line template argument matching...
	{
	  this->assign_elements(e0, eN...);
	}

    /** Construct from a readable_vector and at least one
     * additional element.
     *
     * @note This overload is enabled only if the value_type of @c sub and
     * all of the scalar arguments are convertible to value_type.
     */
    template<class Sub, class E0, class... Elements,
      enable_if_convertible_t<
	value_type, value_type_trait_of_t<Sub>, E0, Elements...>* = nullptr>
	vector(
	  const readable_vector<Sub>& sub, const E0& e0, const Elements&... eN
	  )
	// XXX Should be in vector/fixed_compiled.tpp, but VC++12 has
	// brain-dead out-of-line template argument matching...
	{
	  this->assign(sub, e0, eN...);
	}

    /** Construct from an array type. */
    template<class Array, enable_if_array_t<Array>* = nullptr>
      vector(const Array& array);

    /** Construct from a pointer to an array. */
    template<class Pointer, enable_if_pointer_t<Pointer>* = nullptr>
      vector(const Pointer& array);

    /** Construct from std::initializer_list. */
    template<class Other> vector(std::initializer_list<Other> l);


  public:

    /** Return access to the vector data as a raw pointer. */
    pointer data();

    /** Return const access to the vector data as a raw pointer. */
    const_pointer data() const;

    /** Read-only iterator. */
    const_pointer begin() const;

    /** Read-only iterator. */
    const_pointer end() const;


  public:

    /** Copy assignment. */
    vector_type& operator=(const vector_type& other);

    /** Move assignment. */
    vector_type& operator=(vector_type&& other);

#ifdef CML_HAS_MSVC_BRAIN_DEAD_ASSIGNMENT_OVERLOADS
    template<class Other>
      inline vector_type& operator=(const readable_vector<Other>& other) {
	return this->assign(other);
      }

    template<class Array, enable_if_array_t<Array>* = nullptr>
      inline vector_type& operator=(const Array& array) {
	return this->assign(array);
      }

    template<class Other>
      inline vector_type& operator=(std::initializer_list<Other> l) {
	return this->assign(l);
      }
#endif


  protected:

    /** @name readable_vector Interface */
    /*@{*/

    friend readable_type;

    /** Return the length of the vector. */
    int i_size() const;

    /** Return vector const element @c i. */
    immutable_value i_get(int i) const;

    /*@}*/


  protected:

    /** @name writable_vector Interface */
    /*@{*/

    friend writable_type;

    /** Return vector element @c i. */
    mutable_value i_get(int i);

    /** Set element @c i. */
    template<class Other> vector_type& i_put(int i, const Other& v) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Set element @c i on a temporary. */
    template<class Other> vector_type&& i_put(int i, const Other& v) &&;
#endif

    /*@}*/


  protected:

    /** Fixed-length array. */
    value_type			m_data[Size];
};

} // namespace cml

#ifdef CML_HAS_STRUCTURED_BINDINGS
template<typename E, int Size>
class std::tuple_size<cml::vector<E, cml::fixed<Size>>>
{
public:
  static const int value = Size;
};

template<std::size_t I, typename E, int Size>
class std::tuple_element<I, cml::vector<E, cml::fixed<Size>>>
{
public:
  using type = cml::value_type_of_t<cml::vector<E, cml::fixed<Size>>>;
};
#endif

#define __CML_VECTOR_FIXED_COMPILED_TPP
#include <cml/vector/fixed_compiled.tpp>
#undef __CML_VECTOR_FIXED_COMPILED_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
