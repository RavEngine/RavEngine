/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_fixed_external_h
#define	cml_matrix_fixed_external_h

#include <cml/storage/external_selector.h>
#include <cml/matrix/writable_matrix.h>
#include <cml/matrix/matrix.h>

namespace cml {

template<class Element,
  int Rows, int Cols, typename BasisOrient, typename Layout>
struct matrix_traits<
  matrix<Element, external<Rows,Cols>, BasisOrient, Layout> >
{
  /* The basis must be col_basis or row_basis: */
  static_assert(std::is_same<BasisOrient,row_basis>::value
    || std::is_same<BasisOrient,col_basis>::value, "invalid basis");

  /* Traits and types for the matrix element: */
  typedef scalar_traits<Element>			element_traits;
  typedef typename element_traits::value_type		value_type;
  typedef typename element_traits::pointer		pointer;
  typedef typename element_traits::reference		reference;
  typedef typename element_traits::const_pointer	const_pointer;
  typedef typename element_traits::const_reference	const_reference;
  typedef typename element_traits::mutable_value	mutable_value;
  typedef typename element_traits::immutable_value	immutable_value;

  /* The matrix storage type: */
  typedef rebind_t<external<Rows,Cols>, matrix_storage_tag> storage_type;
  typedef typename storage_type::size_tag		size_tag;
  static_assert(std::is_same<size_tag, fixed_size_tag>::value,
    "invalid size tag");

  /* Array rows (should be positive): */
  static const int array_rows = storage_type::array_rows;
  static_assert(array_rows > 0, "invalid row size");

  /* Array columns (should be positive): */
  static const int array_cols = storage_type::array_cols;
  static_assert(array_cols > 0, "invalid column size");

  /* Basis orientation: */
  typedef BasisOrient					basis_tag;

  /* Layout: */
  typedef Layout					layout_tag;

  /** Constant containing the matrix basis enumeration value. */
  static const basis_kind matrix_basis = basis_tag::value;

  /** Constant containing the array layout enumeration value. */
  static const layout_kind array_layout = layout_tag::value;
};

/** Fixed-size matrix. */
template<class Element,
  int Rows, int Cols, typename BasisOrient, typename Layout>
class matrix<Element, external<Rows,Cols>, BasisOrient, Layout>
: public writable_matrix<
  matrix<Element, external<Rows,Cols>, BasisOrient, Layout>>
{
  public:

    typedef matrix<Element,
	    external<Rows,Cols>, BasisOrient, Layout>	matrix_type;
    typedef readable_matrix<matrix_type>		readable_type;
    typedef writable_matrix<matrix_type>		writable_type;
    typedef matrix_traits<matrix_type>			traits_type;
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
    typedef typename traits_type::basis_tag		basis_tag;
    typedef typename traits_type::layout_tag		layout_tag;


  public:

    /** The matrix data type, depending upon the layout. */
    typedef if_t<layout_tag::value == row_major_c
      , value_type[Rows][Cols]
      , value_type[Cols][Rows]>				matrix_data_type;


  public:

    /* Include methods from writable_type: */
    using writable_type::operator();
#ifndef CML_HAS_MSVC_BRAIN_DEAD_ASSIGNMENT_OVERLOADS
    using writable_type::operator=;
#endif


  public:

    /** Constant containing the number of rows. */
    static const int array_rows = traits_type::array_rows;

    /** Constant containing the number of columns. */
    static const int array_cols = traits_type::array_cols;

    /** Constant containing the matrix basis enumeration value. */
    static const basis_kind matrix_basis = traits_type::matrix_basis;

    /** Constant containing the array layout enumeration value. */
    static const layout_kind array_layout = traits_type::array_layout;


  public:

    /** Default construct with a null pointer.
     *
     * @warning The default constructor is enabled only if the compiler
     * supports rvalue references from *this.
     */
    matrix();

    /** Construct from the wrapped pointer.
     *
     * @note @c data will be referenced using the assigned matrix layout.
     */
    explicit matrix(pointer data);

    /** Construct from a wrapped pointer to a 2D array of values.
     *
     * @note The dimensions of @c array must match those of the matrix,
     * taking the matrix layout into account.  For example, the C-array
     * initializer for a 3x2 external matrix in row-major layout will have
     * dimensions [3][2], but the initializer for a column-major matrix
     * will have dimensions [2][3].
     */
    matrix(matrix_data_type& array);

    /** Move constructor. */
    matrix(matrix_type&& other);


  public:

    /** Return access to the matrix data as a raw pointer. */
    pointer data();

    /** Return const access to the matrix data as a raw pointer. */
    const_pointer data() const;

    /** Read-only iterator over the elements as a 1D array. */
    const_pointer begin() const;

    /** Read-only iterator over the elements as a 1D array. */
    const_pointer end() const;


  public:

    /** Copy assignment. */
    matrix_type& operator=(const matrix_type& other);

    /** Move assignment. */
    matrix_type& operator=(matrix_type&& other);

#ifdef CML_HAS_MSVC_BRAIN_DEAD_ASSIGNMENT_OVERLOADS
    template<class Other>
      inline matrix_type& operator=(const readable_matrix<Other>& other) {
	return this->assign(other);
      }

    template<class Array, enable_if_array_t<Array>* = nullptr>
      inline matrix_type& operator=(const Array& array) {
	return this->assign(array);
      }

    template<class Other, int Rows, int Cols>
      inline matrix_type& operator=(Other const (&array)[Rows][Cols]) {
	return this->assign(array);
      }

    template<class Other>
      inline matrix_type& operator=(std::initializer_list<Other> l) {
	return this->assign(l);
      }
#endif


  protected:

    /** @name readable_matrix Interface */
    /*@{*/

    friend readable_type;

    /** Return the number of rows. */
    int i_rows() const;

    /** Return the number of columns. */
    int i_cols() const;

    /** Return matrix const element @c (i,j). */
    immutable_value i_get(int i, int j) const;

    /*@}*/


  protected:

    /** @name writeable_matrix Interface */
    /*@{*/

    friend writable_type;

    /** Return matrix element @c (i,j). */
    mutable_value i_get(int i, int j);

    /** Set element @c i. */
    template<class Other> matrix_type&
      i_put(int i, int j, const Other& v) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Set element @c i on a temporary. */
    template<class Other> matrix_type&&
      i_put(int i, int j, const Other& v) &&;
#endif

    /*@}*/


  protected:

    /** Row-major access to const or non-const @c M. */
    template<class Matrix> inline static auto s_access(
      Matrix& M, int i, int j, row_major) -> decltype((*M.m_data)[0][0])
    {
      return (*M.m_data)[i][j];
    }

    /** Column-major access to const or non-const @c M. */
    template<class Matrix> inline static auto s_access(
      Matrix& M, int i, int j, col_major) -> decltype((*M.m_data)[0][0])
    {
      return (*M.m_data)[j][i];
    }


  protected:

    /** Wrapped pointer. */
    matrix_data_type*		m_data;
};

} // namespace cml

#define __CML_MATRIX_FIXED_EXTERNAL_TPP
#include <cml/matrix/fixed_external.tpp>
#undef __CML_MATRIX_FIXED_EXTERNAL_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
