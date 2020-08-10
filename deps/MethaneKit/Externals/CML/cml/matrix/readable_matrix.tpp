/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_READABLE_MATRIX_TPP
#error "matrix/readable_matrix.tpp not included correctly"
#endif

#include <cml/matrix/size_checking.h>
#include <cml/matrix/detail/determinant.h>

namespace cml {

/* Public methods: */

template<class DT> const DT&
readable_matrix<DT>::actual() const
{
  return (const DT&) *this;
}

template<class DT> int
readable_matrix<DT>::rows() const
{
  return this->actual().i_rows();
}

template<class DT> int
readable_matrix<DT>::cols() const
{
  return this->actual().i_cols();
}

template<class DT> std::pair<int,int>
readable_matrix<DT>::size() const
{
  return std::make_pair(this->rows(), this->cols());
}

template<class DT> auto
readable_matrix<DT>::get(int i, int j) const -> immutable_value
{
  return this->actual().i_get(i,j);
}

template<class DT> auto
readable_matrix<DT>::operator()(int i, int j) const -> immutable_value
{
  return this->get(i,j);
}

template<class DT> auto
readable_matrix<DT>::basis_element(int i, int j) const -> immutable_value
{
  return this->basis_element(i,j, basis_tag());
}

template<class DT> int
readable_matrix<DT>::basis_count() const
{
  return this->basis_count(basis_tag());
}

template<class DT> int
readable_matrix<DT>::basis_size() const
{
  return this->basis_size(basis_tag());
}

template<class DT> auto
readable_matrix<DT>::determinant() const -> value_type
{
  cml::check_square(*this);
  return detail::determinant(*this, cml::int_c<traits_type::array_rows>());
}

template<class DT> auto
readable_matrix<DT>::trace() const -> value_type
{
  cml::check_square(*this);
  auto result = this->get(0,0);
  for(int i = 1; i < this->rows(); ++ i) result += this->get(i,i);
  return result;
}


/* Internal methods: */

template<class DT> auto
readable_matrix<DT>::basis_element(int i, int j, row_basis) const
-> immutable_value
{
  return this->get(i,j);
}

template<class DT> auto
readable_matrix<DT>::basis_element(int i, int j, col_basis) const
-> immutable_value
{
  return this->get(j,i);
}

template<class DT> int
readable_matrix<DT>::basis_count(row_basis) const
{
  return this->rows();
}

template<class DT> int
readable_matrix<DT>::basis_count(col_basis) const
{
  return this->cols();
}

template<class DT> int
readable_matrix<DT>::basis_size(row_basis) const
{
  return this->cols();
}

template<class DT> int
readable_matrix<DT>::basis_size(col_basis) const
{
  return this->rows();
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
