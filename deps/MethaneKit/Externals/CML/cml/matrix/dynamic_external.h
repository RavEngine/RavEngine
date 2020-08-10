/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_dynamic_external_h
#define	cml_matrix_dynamic_external_h

#include <cml/storage/external_selector.h>
#include <cml/matrix/writable_matrix.h>
#include <cml/matrix/matrix.h>

namespace cml {

template<class Element, typename BasisOrient, typename Layout>
struct matrix_traits<
  matrix<Element, external<>, BasisOrient, Layout> >
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
  typedef rebind_t<external<>, matrix_storage_tag>	storage_type;
  typedef typename storage_type::size_tag		size_tag;
  static_assert(std::is_same<size_tag, dynamic_size_tag>::value,
    "invalid size tag");

  /* Array rows (should be -1): */
  static const int array_rows = storage_type::array_rows;
  static_assert(array_rows == -1, "invalid row size");

  /* Array rows (should be -1): */
  static const int array_cols = storage_type::array_cols;
  static_assert(array_cols == -1, "invalid column size");

  /* Basis orientation: */
  typedef BasisOrient					basis_tag;

  /* Layout: */
  typedef Layout					layout_tag;

  /** Constant containing the array layout enumeration value. */
  static const layout_kind array_layout = layout_tag::value;

  /** Constant containing the matrix basis enumeration value. */
  static const basis_kind matrix_basis = basis_tag::value;
};

/** Fixed-size matrix. */
template<class Element, typename BasisOrient, typename Layout>
class matrix<Element, external<>, BasisOrient, Layout>
: public writable_matrix<
  matrix<Element, external<>, BasisOrient, Layout>>
{
  // The basis must be col_basis or row_basis (NOT is_basis_tag!):
  static_assert(std::is_same<BasisOrient,row_basis>::value
    || std::is_same<BasisOrient,col_basis>::value, "invalid basis");

  // The layout must be col_major or row_major (NOT is_layout_tag!):
  static_assert(std::is_same<Layout,row_major>::value
    || std::is_same<Layout,col_major>::value, "invalid layout");

  public:

    typedef matrix<Element,
	    external<>, BasisOrient, Layout>		matrix_type;
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

    /** Default construct with a null pointer and 0 size (rows, cols).
     *
     * @warning The default constructor is enabled only if the compiler
     * supports rvalue references from *this.
     */
    matrix();

    /** Construct from the wrapped pointer and the dimensions.
     *
     * @note @c data will be referenced using the assigned matrix layout.
     *
     * @note This is for compatibility with CML1.
     */
    matrix(pointer data, int rows, int cols);

    /** Construct from the wrapped pointer and the dimensions.
     *
     * @note @c data will be referenced using the assigned matrix layout.
     */
    matrix(int rows, int cols, pointer data);

    /** Construct from a wrapped pointer to a 2D array of values with
     * dimensions N1xN2.
     *
     * @note The dimensions of @c array must take the matrix layout into
     * account.  For example, the C-array initializer for a 3x2 external
     * matrix in row-major layout will have dimensions N1xN2 = [3][2], but
     * the initializer for a column-major matrix will have dimensions N1xN2
     * = [2][3].
     */
    template<class Other, int N1, int N2> matrix(Other (&array)[N1][N2]);

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

    /** Resize (reshape) the matrix to the specified size.
     *
     * @note The existing elements are not changed.
     *
     * @throws std::invalid_argument if @c rows or @c cols is negative.
     *
     * @throws matrix_size_error if the number of elements in the resized
     * matrix would be different from the original.
     */
    void resize(int rows, int cols);

    /** Reset the matrix to have no elements and no external pointer. */
    void reset();


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
      Matrix& M, int i, int j, row_major) -> decltype(M.m_data[0])
    {
      return M.m_data[i*M.m_cols + j];
    }

    /** Column-major access to const or non-const @c M. */
    template<class Matrix> inline static auto s_access(
      Matrix& M, int i, int j, col_major) -> decltype(M.m_data[0])
    {
      return M.m_data[j*M.m_rows + i];
    }



  protected:

    /** Wrapped pointer. */
    pointer			m_data;

    /** Row count. */
    int				m_rows;

    /** Column count. */
    int				m_cols;
};

} // namespace cml

#define __CML_MATRIX_DYNAMIC_EXTERNAL_TPP
#include <cml/matrix/dynamic_external.tpp>
#undef __CML_MATRIX_DYNAMIC_EXTERNAL_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
