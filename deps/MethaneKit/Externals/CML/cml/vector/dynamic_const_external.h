/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_dynamic_const_external_h
#define	cml_vector_dynamic_const_external_h

#include <cml/storage/external_selector.h>
#include <cml/vector/readable_vector.h>
#include <cml/vector/vector.h>

namespace cml {

template<class Element>
struct vector_traits< vector<const Element, external<>> >
{
  /* Traits and types for the vector element: */
  typedef scalar_traits<Element>			element_traits;
  typedef typename element_traits::value_type		value_type;
  typedef typename element_traits::const_pointer	const_pointer;
  typedef typename element_traits::const_reference	const_reference;
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
class vector<const Element, external<>>
: public readable_vector< vector<const Element, external<>> >
{
  public:

    typedef vector<const Element, external<>>		vector_type;
    typedef readable_vector<vector_type>		readable_type;
    typedef vector_traits<vector_type>			traits_type;
    typedef typename traits_type::element_traits	element_traits;
    typedef typename traits_type::value_type		value_type;
    typedef typename traits_type::const_pointer		const_pointer;
    typedef typename traits_type::const_reference	const_reference;
    typedef typename traits_type::immutable_value	immutable_value;
    typedef typename traits_type::storage_type		storage_type;
    typedef typename traits_type::size_tag		size_tag;


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

    /** Construct from the wrapped pointer and size. */
    vector(const_pointer data, int size);

    /** Construct from the wrapped pointer and size. */
    vector(int size, const_pointer data);

    /** Move constructor. */
    vector(vector_type&& other);


  public:

    /** Return const access to the vector data as a raw pointer. */
    const_pointer data() const;

    /** Read-only iterator. */
    const_pointer begin() const;

    /** Read-only iterator. */
    const_pointer end() const;

    /** Reset the vector to have no elements and no external pointer. */
    void reset();


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

    /** Wrapped pointer. */
    const_pointer		m_data;

    /** Number of elements. */
    int				m_size;
};

} // namespace cml

#define __CML_VECTOR_DYNAMIC_CONST_EXTERNAL_TPP
#include <cml/vector/dynamic_const_external.tpp>
#undef __CML_VECTOR_DYNAMIC_CONST_EXTERNAL_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
