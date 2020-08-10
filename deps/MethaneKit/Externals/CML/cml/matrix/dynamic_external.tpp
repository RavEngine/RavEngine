/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_DYNAMIC_EXTERNAL_TPP
#error "matrix/dynamic_external.tpp not included correctly"
#endif

namespace cml {

/* dynamic_external 'structors: */

template<class E, typename BO, typename L>
matrix<E, external<>, BO, L>::matrix()
: m_data(0), m_rows(0), m_cols(0)
{
#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
  static_assert(false, "external matrix default constructor not supported");
#endif
}

template<class E, typename BO, typename L>
matrix<E, external<>, BO, L>::matrix(pointer data, int rows, int cols)
: m_data(data), m_rows(rows), m_cols(cols)
{
  cml_require(rows >= 0, std::invalid_argument, "rows < 0");
  cml_require(cols >= 0, std::invalid_argument, "cols < 0");
}

template<class E, typename BO, typename L>
matrix<E, external<>, BO, L>::matrix(int rows, int cols, pointer data)
: m_data(data), m_rows(rows), m_cols(cols)
{
  cml_require(rows >= 0, std::invalid_argument, "rows < 0");
  cml_require(cols >= 0, std::invalid_argument, "cols < 0");
}

template<class E, typename BO, typename L>
template<class Other, int N1, int N2>
matrix<E, external<>, BO, L>::matrix(Other (&array)[N1][N2])
: m_data(&array[0][0])
, m_rows(array_layout == row_major_c ? N1 : N2)
, m_cols(array_layout == row_major_c ? N2 : N1)
{
}

template<class E, typename BO, typename L>
matrix<E, external<>, BO, L>::matrix(matrix_type&& other)
{
  this->operator=(std::move(other));
}




/* Public methods: */

template<class E, typename BO, typename L> auto
matrix<E, external<>, BO, L>::data() -> pointer
{
  return this->m_data;
}

template<class E, typename BO, typename L> auto
matrix<E, external<>, BO, L>::data() const -> const_pointer
{
  return this->m_data;
}

template<class E, typename BO, typename L> auto
matrix<E, external<>, BO, L>::begin() const -> const_pointer
{
  return this->m_data;
}

template<class E, typename BO, typename L> auto
matrix<E, external<>, BO, L>::end() const -> const_pointer
{
  return this->m_data + this->m_rows*this->m_cols;
}

template<class E, typename BO, typename L> void
matrix<E, external<>, BO, L>::resize(int rows, int cols)
{
  cml_require(rows >= 0, std::invalid_argument, "rows < 0");
  cml_require(cols >= 0, std::invalid_argument, "cols < 0");
  cml::check_linear_size(*this, rows*cols);
  this->m_rows = rows;
  this->m_cols = cols;
}

template<class E, typename BO, typename L> void
matrix<E, external<>, BO, L>::reset()
{
  this->m_rows = this->m_cols = 0;
  this->m_data = nullptr;
}


template<class E, typename BO, typename L> auto
matrix<E, external<>, BO, L>::operator=(const matrix_type& other)
-> matrix_type&
{
  return this->assign(other);
}

template<class E, typename BO, typename L> auto
matrix<E, external<>, BO, L>::operator=(matrix_type&& other)
-> matrix_type&
{
  this->m_data = other.m_data;
  this->m_rows = other.m_rows;
  this->m_cols = other.m_cols;
  other.m_data = nullptr;
  other.m_rows = 0;
  other.m_cols = 0;
  return *this;
}



/* Internal methods: */

/* readable_matrix interface: */

template<class E, typename BO, typename L> int
matrix<E, external<>, BO, L>::i_rows() const
{
  return this->m_rows;
}

template<class E, typename BO, typename L> int
matrix<E, external<>, BO, L>::i_cols() const
{
  return this->m_cols;
}

template<class E, typename BO, typename L> auto
matrix<E, external<>, BO, L>::i_get(int i, int j) const -> immutable_value
{
  return s_access(*this, i, j, layout_tag());
}


/* writable_matrix interface: */

template<class E, typename BO, typename L> auto
matrix<E, external<>, BO, L>::i_get(int i, int j) -> mutable_value
{
  return s_access(*this, i, j, layout_tag());
}

template<class E, typename BO, typename L>
template<class Other> auto matrix<E, external<>, BO, L>::i_put(
  int i, int j, const Other& v
  ) __CML_REF -> matrix_type&
{
  s_access(*this, i, j, layout_tag()) = value_type(v);
  return *this;
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class E, typename BO, typename L>
template<class Other> auto matrix<E, external<>, BO, L>::i_put(
  int i, int j, const Other& v
  ) && -> matrix_type&&
{
  s_access(*this, i, j, layout_tag()) = value_type(v);
  return (matrix_type&&) *this;
}
#endif

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
