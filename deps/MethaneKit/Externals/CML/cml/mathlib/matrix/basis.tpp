/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATHLIB_MATRIX_BASIS_TPP
#error "mathlib/matrix/basis.tpp not included correctly"
#endif

#include <cml/matrix/row_col.h>
#include <cml/matrix/writable_matrix.h>
#include <cml/mathlib/matrix/size_checking.h>

namespace cml {

/* 2D basis functions: */

template<class Sub1, class Sub2> inline void
matrix_set_basis_vector_2D(
  writable_matrix<Sub1>& m, int i, const readable_vector<Sub2>& v
  )
{
  cml::check_linear_2D(m);
  cml::check_size(v, int_c<2>());
  cml_require(0 <= i && i <= 1, std::invalid_argument, "invalid 2D index");

  m.set_basis_element(i, 0, v[0]);
  m.set_basis_element(i, 1, v[1]);
}

template<class Sub1, class Sub2> inline void
matrix_set_x_basis_vector_2D(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& x
  )
{
  matrix_set_basis_vector_2D(m, 0, x);
}

template<class Sub1, class Sub2> inline void
matrix_set_y_basis_vector_2D(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& y
  )
{
  matrix_set_basis_vector_2D(m, 1, y);
}

template<class Sub1, class SubX, class SubY> inline void
matrix_set_basis_vectors_2D(
  writable_matrix<Sub1>& m,
  const readable_vector<SubX>& x,
  const readable_vector<SubY>& y
  )
{
  matrix_set_x_basis_vector_2D(m, x);
  matrix_set_y_basis_vector_2D(m, y);
}


template<class Sub1, class Sub2> inline void
matrix_set_transposed_basis_vector_2D(
  writable_matrix<Sub1>& m, int i, const readable_vector<Sub2>& v
  )
{
  cml::check_linear_2D(m);
  cml::check_size(v, int_c<2>());
  cml_require(0 <= i && i <= 1, std::invalid_argument, "invalid 2D index");

  m.set_basis_element(0, i, v[0]);
  m.set_basis_element(1, i, v[1]);
}

template<class Sub1, class Sub2> inline void
matrix_set_transposed_x_basis_vector_2D(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& x
  )
{
  matrix_set_transposed_basis_vector_2D(m, 0, x);
}

template<class Sub1, class Sub2> inline void
matrix_set_transposed_y_basis_vector_2D(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& y
  )
{
  matrix_set_transposed_basis_vector_2D(m, 1, y);
}

template<class Sub1, class SubX, class SubY> inline void
matrix_set_transposed_basis_vectors_2D(
  writable_matrix<Sub1>& m,
  const readable_vector<SubX>& x,
  const readable_vector<SubY>& y
  )
{
  matrix_set_transposed_x_basis_vector_2D(m, x);
  matrix_set_transposed_y_basis_vector_2D(m, y);
}


template<class Sub> inline auto
matrix_get_basis_vector_2D(const readable_matrix<Sub>& m, int i)
-> n_basis_vector_of_t<Sub,2>
{
  cml::check_linear_2D(m);
  cml_require(0 <= i && i <= 1, std::invalid_argument, "invalid 2D index");
  return n_basis_vector_of_t<Sub,2>(
    m.basis_element(i,0), m.basis_element(i,1));
}

template<class Sub> inline auto
matrix_get_x_basis_vector_2D(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,2>
{
  return matrix_get_basis_vector_2D(m,0);
}

template<class Sub> inline auto
matrix_get_y_basis_vector_2D(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,2>
{
  return matrix_get_basis_vector_2D(m,1);
}

template<class Sub, class SubX, class SubY> inline void
matrix_get_basis_vectors_2D(
  const readable_matrix<Sub>& m,
  writable_vector<SubX>& x,
  writable_vector<SubY>& y
  )
{
  x = matrix_get_x_basis_vector_2D(m);
  y = matrix_get_y_basis_vector_2D(m);
}


template<class Sub> inline auto
matrix_get_transposed_basis_vector_2D(const readable_matrix<Sub>& m, int i)
-> n_basis_vector_of_t<Sub,2>
{
  cml::check_linear_2D(m);
  cml_require(0 <= i && i <= 1, std::invalid_argument, "invalid 2D index");
  return n_basis_vector_of_t<Sub,2>(
    m.basis_element(0,i), m.basis_element(1,i));
}

template<class Sub> inline auto
matrix_get_transposed_x_basis_vector_2D(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,2>
{
  return matrix_get_transposed_basis_vector_2D(m,0);
}

template<class Sub> inline auto
matrix_get_transposed_y_basis_vector_2D(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,2>
{
  return matrix_get_transposed_basis_vector_2D(m,1);
}

template<class Sub, class SubX, class SubY> inline void
matrix_get_transposed_basis_vectors_2D(
  const readable_matrix<Sub>& m,
  writable_vector<SubX>& x,
  writable_vector<SubY>& y
  )
{
  x = matrix_get_transposed_x_basis_vector_2D(m);
  y = matrix_get_transposed_y_basis_vector_2D(m);
}



/* 3D basis functions: */

template<class Sub1, class Sub2> inline void
matrix_set_basis_vector(
  writable_matrix<Sub1>& m, int i, const readable_vector<Sub2>& v
  )
{
  cml::check_linear_3D(m);
  cml::check_size(v, int_c<3>());
  cml_require(0 <= i && i <= 2, std::invalid_argument, "invalid 3D index");

  m.set_basis_element(i, 0, v[0]);
  m.set_basis_element(i, 1, v[1]);
  m.set_basis_element(i, 2, v[2]);
}

template<class Sub1, class Sub2> inline void
matrix_set_x_basis_vector(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& x
  )
{
  matrix_set_basis_vector(m, 0, x);
}

template<class Sub1, class Sub2> inline void
matrix_set_y_basis_vector(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& y
  )
{
  matrix_set_basis_vector(m, 1, y);
}

template<class Sub1, class Sub2> inline void
matrix_set_z_basis_vector(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& z
  )
{
  matrix_set_basis_vector(m, 2, z);
}

template<class Sub1, class SubX, class SubY, class SubZ> inline void
matrix_set_basis_vectors(
  writable_matrix<Sub1>& m,
  const readable_vector<SubX>& x,
  const readable_vector<SubY>& y,
  const readable_vector<SubZ>& z
  )
{
  matrix_set_x_basis_vector(m, x);
  matrix_set_y_basis_vector(m, y);
  matrix_set_z_basis_vector(m, z);
}


template<class Sub1, class Sub2> inline void
matrix_set_transposed_basis_vector(
  writable_matrix<Sub1>& m, int i, const readable_vector<Sub2>& v
  )
{
  cml::check_linear_3D(m);
  cml::check_size(v, int_c<3>());
  cml_require(0 <= i && i <= 2, std::invalid_argument, "invalid 3D index");

  m.set_basis_element(0, i, v[0]);
  m.set_basis_element(1, i, v[1]);
  m.set_basis_element(2, i, v[2]);
}

template<class Sub1, class Sub2> inline void
matrix_set_transposed_x_basis_vector(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& x
  )
{
  matrix_set_transposed_basis_vector(m, 0, x);
}

template<class Sub1, class Sub2> inline void
matrix_set_transposed_y_basis_vector(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& y
  )
{
  matrix_set_transposed_basis_vector(m, 1, y);
}

template<class Sub1, class Sub2> inline void
matrix_set_transposed_z_basis_vector(
  writable_matrix<Sub1>& m, const readable_vector<Sub2>& z
  )
{
  matrix_set_transposed_basis_vector(m, 2, z);
}

template<class Sub1, class SubX, class SubY, class SubZ> inline void
matrix_set_transposed_basis_vectors(
  writable_matrix<Sub1>& m,
  const readable_vector<SubX>& x,
  const readable_vector<SubY>& y,
  const readable_vector<SubZ>& z
  )
{
  matrix_set_transposed_x_basis_vector(m, x);
  matrix_set_transposed_y_basis_vector(m, y);
  matrix_set_transposed_z_basis_vector(m, z);
}


template<class Sub> inline auto
matrix_get_basis_vector(const readable_matrix<Sub>& m, int i)
-> n_basis_vector_of_t<Sub,3>
{
  cml::check_linear_3D(m);
  cml_require(0 <= i && i <= 2, std::invalid_argument, "invalid 3D index");
  return n_basis_vector_of_t<Sub,3>(
    m.basis_element(i,0), m.basis_element(i,1), m.basis_element(i,2));
}

template<class Sub> inline auto
matrix_get_x_basis_vector(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,3>
{
  return matrix_get_basis_vector(m,0);
}

template<class Sub> inline auto
matrix_get_y_basis_vector(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,3>
{
  return matrix_get_basis_vector(m,1);
}

template<class Sub> inline auto
matrix_get_z_basis_vector(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,3>
{
  return matrix_get_basis_vector(m,2);
}

template<class Sub, class SubX, class SubY, class SubZ> inline void
matrix_get_basis_vectors(
  const readable_matrix<Sub>& m,
  writable_vector<SubX>& x,
  writable_vector<SubY>& y,
  writable_vector<SubZ>& z
  )
{
  x = matrix_get_x_basis_vector(m);
  y = matrix_get_y_basis_vector(m);
  z = matrix_get_z_basis_vector(m);
}


template<class Sub> inline auto
matrix_get_transposed_basis_vector(const readable_matrix<Sub>& m, int i)
-> n_basis_vector_of_t<Sub,3>
{
  cml::check_linear_3D(m);
  cml_require(0 <= i && i <= 2, std::invalid_argument, "invalid 3D index");
  return n_basis_vector_of_t<Sub,3>(
    m.basis_element(0,i), m.basis_element(1,i), m.basis_element(2,i));
}

template<class Sub> inline auto
matrix_get_transposed_x_basis_vector(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,3>
{
  return matrix_get_transposed_basis_vector(m,0);
}

template<class Sub> inline auto
matrix_get_transposed_y_basis_vector(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,3>
{
  return matrix_get_transposed_basis_vector(m,1);
}

template<class Sub> inline auto
matrix_get_transposed_z_basis_vector(const readable_matrix<Sub>& m)
-> n_basis_vector_of_t<Sub,3>
{
  return matrix_get_transposed_basis_vector(m,2);
}

template<class Sub, class SubX, class SubY, class SubZ> inline void
matrix_get_transposed_basis_vectors(
  const readable_matrix<Sub>& m,
  writable_vector<SubX>& x,
  writable_vector<SubY>& y,
  writable_vector<SubZ>& z)
{
  x = matrix_get_transposed_x_basis_vector(m);
  y = matrix_get_transposed_y_basis_vector(m);
  z = matrix_get_transposed_z_basis_vector(m);
}



/* nD basis functions: */

namespace detail {

template<class Sub> inline auto
matrix_get_basis_vector(const readable_matrix<Sub>& m, int i, row_basis)
-> basis_vector_of_t<Sub>
{
  return cml::row(m,i);
}

template<class Sub> inline auto
matrix_get_basis_vector(const readable_matrix<Sub>& m, int i, col_basis)
-> basis_vector_of_t<Sub>
{
  return cml::col(m,i);
}

} // namespace detail

template<class Sub> auto
matrix_get_basis_vector_nD(const readable_matrix<Sub>& m, int i)
-> basis_vector_of_t<Sub>
{
  typedef basis_tag_of_t<Sub> tag;
  return detail::matrix_get_basis_vector(m, i, tag());
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
