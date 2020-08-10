/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_fixed_external_h
#define	cml_vector_fixed_external_h

#include <cml/storage/external_selector.h>
#include <cml/vector/writable_vector.h>
#include <cml/vector/vector.h>

/* Need const specializations with non-const for proper type trait
 * resolution:
 */
#include <cml/vector/fixed_const_external.h>

namespace cml {

template<class Element, int Size>
struct vector_traits< vector<Element, external<Size>> >
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
  typedef rebind_t<external<Size>, vector_storage_tag>	storage_type;
  typedef typename storage_type::size_tag		size_tag;
  static_assert(std::is_same<size_tag, fixed_size_tag>::value,
    "invalid size tag");

  /* Array size (should be positive): */
  static const int array_size = storage_type::array_size;
  static_assert(array_size > 0, "invalid vector size");
};

/** Fixed-length wrapped array pointer as a vector. */
template<class Element, int Size>
class vector<Element, external<Size>>
: public writable_vector< vector<Element, external<Size>> >
{
  public:

    typedef vector<Element, external<Size>>		vector_type;
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

    /** Default construct with a null pointer. */
    vector();

    /** Construct from the wrapped pointer. */
    explicit vector(pointer data);

    /** Copy constructor.
     *
     * @warning This copy has the semantics of a raw pointer, and can lead
     * to memory leaks if not used correctly.
     */
    vector(const vector_type& other);

    /** Move constructor. */
    vector(vector_type&& other);


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

    /** Copy assignment.
     *
     * @warning This assignment has the semantics of a raw pointer, and can
     * lead to memory leaks if not used correctly.
     */
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

    /** Wrapped pointer. */
    pointer			m_data;
};

} // namespace cml

#ifdef CML_HAS_STRUCTURED_BINDINGS
template<typename E, int Size>
class std::tuple_size<cml::vector<E, cml::external<Size>>>
{
public:
  static const int value = Size;
};

template<std::size_t I, typename E, int Size>
class std::tuple_element<I, cml::vector<E, cml::external<Size>>>
{
public:
  using type = cml::value_type_of_t<cml::vector<E, cml::external<Size>>>;
};
#endif

#define __CML_VECTOR_FIXED_EXTERNAL_TPP
#include <cml/vector/fixed_external.tpp>
#undef __CML_VECTOR_FIXED_EXTERNAL_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
