/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_FIXED_EXTERNAL_TPP
#error "matrix/fixed_external.tpp not included correctly"
#endif

namespace cml {

/* fixed_external 'structors: */

template<class E, int R, int C, typename BO, typename L>
matrix<E, external<R,C>, BO, L>::matrix()
: m_data(0)
{
#ifndef CML_HAS_RVALUE_REFERENCE_FROM_THIS
  static_assert(false, "external matrix default constructor not supported");
#endif
}

template<class E, int R, int C, typename BO, typename L>
matrix<E, external<R,C>, BO, L>::matrix(pointer data)
: m_data(reinterpret_cast<matrix_data_type*>(data))
{
}

template<class E, int R, int C, typename BO, typename L>
matrix<E, external<R,C>, BO, L>::matrix(matrix_data_type& array)
: m_data(&array)
{
}

template<class E, int R, int C, typename BO, typename L>
matrix<E, external<R,C>, BO, L>::matrix(matrix_type&& other)
{
  this->operator=(std::move(other));
}




/* Public methods: */

template<class E, int R, int C, typename BO, typename L> auto
matrix<E, external<R,C>, BO, L>::data() -> pointer
{
  return reinterpret_cast<pointer>(this->m_data);
}

template<class E, int R, int C, typename BO, typename L> auto
matrix<E, external<R,C>, BO, L>::data() const -> const_pointer
{
  return reinterpret_cast<pointer>(this->m_data);
}

template<class E, int R, int C, typename BO, typename L> auto
matrix<E, external<R,C>, BO, L>::begin() const -> const_pointer
{
  return reinterpret_cast<pointer>(this->m_data);
}

template<class E, int R, int C, typename BO, typename L> auto
matrix<E, external<R,C>, BO, L>::end() const -> const_pointer
{
  return reinterpret_cast<pointer>(this->m_data) + R*C;
}


template<class E, int R, int C, typename BO, typename L> auto
matrix<E, external<R,C>, BO, L>::operator=(const matrix_type& other)
-> matrix_type&
{
  return this->assign(other);
}

template<class E, int R, int C, typename BO, typename L> auto
matrix<E, external<R,C>, BO, L>::operator=(matrix_type&& other)
-> matrix_type&
{
  this->m_data = other.m_data;
  other.m_data = nullptr;
  return *this;
}


/* Internal methods: */

/* readable_matrix interface: */

template<class E, int R, int C, typename BO, typename L> int
matrix<E, external<R,C>, BO, L>::i_rows() const
{
  return R;
}

template<class E, int R, int C, typename BO, typename L> int
matrix<E, external<R,C>, BO, L>::i_cols() const
{
  return C;
}

template<class E, int R, int C, typename BO, typename L> auto
matrix<E, external<R,C>, BO, L>::i_get(int i, int j) const -> immutable_value
{
  return s_access(*this, i, j, layout_tag());
}


/* writable_matrix interface: */

template<class E, int R, int C, typename BO, typename L> auto
matrix<E, external<R,C>, BO, L>::i_get(int i, int j) -> mutable_value
{
  return s_access(*this, i, j, layout_tag());
}

template<class E, int R, int C, typename BO, typename L>
template<class Other> auto matrix<E, external<R,C>, BO, L>::i_put(
  int i, int j, const Other& v
  ) __CML_REF -> matrix_type&
{
  s_access(*this, i, j, layout_tag()) = value_type(v);
  return *this;
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class E, int R, int C, typename BO, typename L>
template<class Other> auto matrix<E, external<R,C>, BO, L>::i_put(
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
