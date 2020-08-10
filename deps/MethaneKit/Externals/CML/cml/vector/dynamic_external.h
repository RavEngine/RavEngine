/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_dynamic_external_h
#define	cml_vector_dynamic_external_h

#include <cml/storage/external_selector.h>
#include <cml/vector/writable_vector.h>
#include <cml/vector/vector.h>

/* Need const specializations with non-const for proper type trait
 * resolution:
 */
#include <cml/vector/dynamic_const_external.h>

namespace cml {

template<class Element>
struct vector_traits< vector<Element, external<>> >
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
  typedef rebind_t<external<>, vector_storage_tag>	storage_type;
  typedef typename storage_type::size_tag		size_tag;
  static_assert(std::is_same<size_tag, dynamic_size_tag>::value,
    "invalid size tag");

  /* Array size (should be -1): */
  static const int array_size = storage_type::array_size;
  static_assert(array_size == -1, "invalid vector size");
};

/** Runtime-length wrapped array pointer as a vector. */
template<class Element>
class vector<Element, external<>>
: public writable_vector< vector<Element, external<>> >
{
  public:

    typedef vector<Element, external<>>			vector_type;
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
    static const int array_size = -1;


  public:

    /** Default construct with a null pointer and 0 size.
     *
     * @warning The default constructor is enabled only if the compiler
     * supports rvalue references from *this.
     */
    vector();

    /** Construct from the wrapped pointer and size.
     *
     * @note This is for CML1 compatibility.
     */
    vector(pointer data, int size);

    /** Construct from the wrapped pointer and size. */
    vector(int size, pointer data);

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

    /** Reset the vector to have no elements and no external pointer. */
    void reset();


  public:

    /** Copy assignment.
     *
     * @note This copies element-by-element from @c other.
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

    /** Number of elements. */
    int				m_size;
};

} // namespace cml

#define __CML_VECTOR_DYNAMIC_EXTERNAL_TPP
#include <cml/vector/dynamic_external.tpp>
#undef __CML_VECTOR_DYNAMIC_EXTERNAL_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
