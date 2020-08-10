/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_WRITABLE_MATRIX_TPP
#error "matrix/writable_matrix.tpp not included correctly"
#endif

#include <random>
#include <cml/scalar/binary_ops.h>
#include <cml/vector/readable_vector.h>
#include <cml/matrix/detail/check_or_resize.h>
#include <cml/matrix/detail/copy.h>
#include <cml/matrix/detail/apply.h>
#include <cml/matrix/detail/generate.h>
#include <cml/matrix/detail/transpose.h>
#include <cml/matrix/detail/inverse.h>

namespace cml {
namespace detail {

/* Terminate the assignment recursion at the final element. */
template<int I, class Sub, class E0> inline void
assign_elements(writable_matrix<Sub>& sub, const E0& e0)
{
  sub.put(I / sub.cols(), I % sub.cols(), e0);
}

/* Set element (I/cols(),I%cols()) of sub to e0, then assign the remainder
 * of the elements starting from I+1.
 */
template<int I, class Sub, class E0, class... Es>
inline void assign_elements(
  writable_matrix<Sub>& sub, const E0& e0, const Es&... eN
  )
{
  sub.put(I / sub.cols(), I % sub.cols(), e0);
  assign_elements<I+1>(sub, eN...);
}

/* Assign elements of @c sub from @c eN, assuming @c eN is given in
 * row-major order.
 */
template<class Sub, class... Es> inline void
assign_elements(writable_matrix<Sub>& sub, const Es&... eN)
{
  assign_elements<0>(sub, eN...);
}

} // namespace detail



/* Public methods: */

template<class DT> DT&
writable_matrix<DT>::actual()
{
  return (DT&) *this;
}

template<class DT> auto
writable_matrix<DT>::get(int i, int j) -> mutable_value
{
  return this->actual().i_get(i,j);
}

template<class DT> template<class Other> DT&
writable_matrix<DT>::put(int i, int j, const Other& v) __CML_REF
{
  return this->actual().i_put(i,j,v);
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class Other> DT&&
writable_matrix<DT>::put(int i, int j, const Other& v) &&
{
  this->put(i,j,v);		// Forward to put(...) &
  return (DT&&) *this;
}
#endif

template<class DT> auto
writable_matrix<DT>::operator()(int i, int j) -> mutable_value
{
  return this->get(i,j);
}


template<class DT> template<class Other> DT&
writable_matrix<DT>::set_basis_element(int i, int j, const Other& v) __CML_REF
{
  this->set_basis_element(i, j, v, basis_tag());
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class Other> DT&&
writable_matrix<DT>::set_basis_element(int i, int j, const Other& v) &&
{
  this->set_basis_element(i,j,v);	// Forward to set_basis_element(...) &
  return (DT&&) *this;
}
#endif


template<class DT> template<class Sub> DT&
writable_matrix<DT>::set_row(int i, const readable_vector<Sub>& v) __CML_REF
{
  cml::check_same_col_size(*this, v);
  for(int j = 0; j < this->cols(); ++ j) this->put(i,j, v.get(j));
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class Sub> DT&&
writable_matrix<DT>::set_row(int i, const readable_vector<Sub>& v) &&
{
  this->set_row(i, v);
  return (DT&&) *this;
}
#endif

template<class DT> template<class Sub> DT&
writable_matrix<DT>::set_col(int j, const readable_vector<Sub>& v) __CML_REF
{
  cml::check_same_row_size(*this, v);
  for(int i = 0; i < this->rows(); ++ i) this->put(i,j, v.get(i));
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class Sub> DT&&
writable_matrix<DT>::set_col(int j, const readable_vector<Sub>& v) &&
{
  this->set_col(j, v);
  return (DT&&) *this;
}
#endif


template<class DT> DT&
writable_matrix<DT>::zero() __CML_REF
{
  auto zero_f = [](int,int) { return value_type(0); };
  detail::generate(*this, zero_f, layout_tag());
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> DT&&
writable_matrix<DT>::zero() &&
{
  this->zero();			// Forward to zero &
  return (DT&&) *this;
}
#endif


template<class DT> DT&
writable_matrix<DT>::identity() __CML_REF
{
  auto identity_f = [](int i, int j) { return value_type(i == j); };
  detail::generate(*this, identity_f, layout_tag());
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> DT&&
writable_matrix<DT>::identity() &&
{
  this->identity();		// Forward to zero &
  return (DT&&) *this;
}
#endif

template<class DT> DT&
writable_matrix<DT>::random(
  const_reference low, const_reference high
  ) __CML_REF
{
  typedef if_t<std::is_integral<value_type>::value
    , std::uniform_int_distribution<value_type>
    , std::uniform_real_distribution<value_type>>	distribution_type;

  std::random_device rd;	// Non-deterministic seed, if supported.
  std::default_random_engine gen(rd());
  distribution_type d(low, high);
  auto random_f = [&d,&gen](int,int) { return d(gen); };
  detail::generate(*this, random_f, layout_tag());
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> DT&&
writable_matrix<DT>::random(const_reference low, const_reference high) &&
{
  this->random(low, high);
  return (DT&&) *this;
}
#endif

template<class DT> DT&
writable_matrix<DT>::fill(const_reference v) __CML_REF
{
  detail::generate(*this, [&v](int,int) { return v; }, layout_tag());
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> DT&&
writable_matrix<DT>::fill(const_reference v) &&
{
  this->fill(v);
  return (DT&&) *this;
}
#endif

template<class DT> DT&
writable_matrix<DT>::inverse() __CML_REF
{
  cml::check_square(*this);
  detail::inverse(*this, cml::int_c<traits_type::array_rows>());
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> DT&&
writable_matrix<DT>::inverse() &&
{
  this->inverse();
  return (DT&&) *this;
}
#endif

template<class DT> DT&
writable_matrix<DT>::transpose() __CML_REF
{
  typedef size_tag_of_t<traits_type> tag;
  detail::transpose(*this, tag());
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> DT&&
writable_matrix<DT>::transpose() &&
{
  this->transpose();
  return (DT&&) *this;
}
#endif


template<class DT> template<class ODT> DT&
writable_matrix<DT>::operator=(const readable_matrix<ODT>& other) __CML_REF
{
  return this->assign(other);
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class ODT> DT&&
writable_matrix<DT>::operator=(const readable_matrix<ODT>& other) &&
{
  this->operator=(other);
  return (DT&&) *this;
}
#endif

template<class DT> template<class Array, enable_if_array_t<Array>*> DT&
writable_matrix<DT>::operator=(const Array& array) __CML_REF
{
  return this->assign(array);
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class Array, enable_if_array_t<Array>*> DT&&
writable_matrix<DT>::operator=(const Array& array) &&
{
  this->operator=(array);
  return (DT&&) *this;
}
#endif

template<class DT>
template<class Other, int Rows, int Cols> DT&
writable_matrix<DT>::operator=(Other const (&array)[Rows][Cols]) __CML_REF
{
  return this->assign(array);
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT>
template<class Other, int Rows, int Cols> DT&
writable_matrix<DT>::operator=(Other const (&array)[Rows][Cols]) &&
{
  this->operator=(array);
  return (DT&&) *this;
}
#endif

template<class DT> template<class Other> DT&
writable_matrix<DT>::operator=(std::initializer_list<Other> l) __CML_REF
{
  return this->assign(l);
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class Other> DT&&
writable_matrix<DT>::operator=(std::initializer_list<Other> l) &&
{
  return this->assign(l);
}
#endif

template<class DT> template<class ODT> DT&
writable_matrix<DT>::operator+=(const readable_matrix<ODT>& other) __CML_REF
{
  detail::check_or_resize(*this, other);
  detail::apply< binary_plus_t<DT, ODT> >(*this, other, layout_tag());
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class ODT> DT&&
writable_matrix<DT>::operator+=(const readable_matrix<ODT>& other) &&
{
  this->operator+=(other);
  return (DT&&) *this;
}
#endif

template<class DT> template<class ODT> DT&
writable_matrix<DT>::operator-=(const readable_matrix<ODT>& other) __CML_REF
{
  detail::check_or_resize(*this, other);
  detail::apply< binary_minus_t<DT, ODT> >(*this, other, layout_tag());
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT> template<class ODT> DT&&
writable_matrix<DT>::operator-=(const readable_matrix<ODT>& other) &&
{
  this->operator-=(other);
  return (DT&&) *this;
}
#endif


template<class DT>
template<class ScalarT, typename enable_if_convertible<
    typename matrix_traits<DT>::value_type, ScalarT>::type*>
DT&
writable_matrix<DT>::operator*=(const ScalarT& v) __CML_REF
{
  detail::apply< binary_multiply_t<DT, ScalarT> >(*this, v, layout_tag());
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT>
template<class ScalarT, typename enable_if_convertible<
    typename matrix_traits<DT>::value_type, ScalarT>::type*>
DT&&
writable_matrix<DT>::operator*=(const ScalarT& v) &&
{
  this->operator*=(v);
  return (DT&&) *this;
}
#endif

template<class DT>
template<class ScalarT, typename enable_if_convertible<
    typename matrix_traits<DT>::value_type, ScalarT>::type*>
DT&
writable_matrix<DT>::operator/=(const ScalarT& v) __CML_REF
{
  detail::apply< binary_divide_t<DT, ScalarT> >(*this, v, layout_tag());
  return this->actual();
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class DT>
template<class ScalarT, typename enable_if_convertible<
    typename matrix_traits<DT>::value_type, ScalarT>::type*>
DT&&
writable_matrix<DT>::operator/=(const ScalarT& v) &&
{
  this->operator/=(v);
  return (DT&&) *this;
}
#endif



/* Internal methods: */

template<class DT> template<class ODT> DT&
writable_matrix<DT>::assign(const readable_matrix<ODT>& other)
{
  detail::check_or_resize(*this, other);
  detail::copy(*this, other, layout_tag());
  return this->actual();
}

template<class DT> template<class Array, enable_if_array_t<Array>*> DT&
writable_matrix<DT>::assign(const Array& array)
{
  cml::check_same_linear_size(*this, array);
  int cols = this->cols();
  for(int i = 0; i < array_size_of_c<Array>::value; ++ i) {
    this->put(i/cols, i%cols, array[i]);
  }
  return this->actual();
}

template<class DT> template<class Other, int R, int C> DT&
writable_matrix<DT>::assign(Other const (&array)[R][C])
{
  detail::check_or_resize(*this, array);
  detail::copy(*this, array, layout_tag());
  return this->actual();
}

template<class DT>
template<class Pointer, enable_if_pointer_t<Pointer>*> DT&
writable_matrix<DT>::assign(const Pointer& array)
{
  int rows = this->rows(), cols = this->cols();
  for(int i = 0; i < rows*cols; ++ i)
    this->put(i/cols, i%cols, array[i]);
  return this->actual();
}

template<class DT> template<class Other> DT&
writable_matrix<DT>::assign(const std::initializer_list<Other>& l)
{
  cml::check_same_linear_size(*this, l);
  int cols = this->cols(), i = 0;
  for(Other v : l) { this->put(i/cols, i%cols, v); ++ i; }
  return this->actual();
}

template<class DT> template<class... Es> DT&
writable_matrix<DT>::assign_elements(const Es&... eN)
{
  static const int N = int(sizeof...(eN));
  cml::check_linear_size(*this, cml::int_c<N>());

  /* Assign elements: */
  detail::assign_elements(*this, eN...);

  /* Done: */
  return this->actual();
}


template<class DT> template<class Other> void
writable_matrix<DT>::set_basis_element(
  int i, int j, const Other& v, row_basis
  )
{
  this->put(i, j, v);
}

template<class DT> template<class Other> void
writable_matrix<DT>::set_basis_element(
  int i, int j, const Other& v, col_basis
  )
{
  this->put(j, i, v);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
