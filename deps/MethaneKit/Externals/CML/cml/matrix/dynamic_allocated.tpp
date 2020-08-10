/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_dynamic_allocated_tpp
#define	cml_matrix_dynamic_allocated_tpp

#ifndef __CML_MATRIX_DYNAMIC_ALLOCATED_TPP
#error "matrix/dynamic_allocated.tpp not included correctly"
#endif

#include <cml/common/exception.h>

namespace cml {

/* dynamic 'structors: */

template<class E, class A, typename BO, typename L>
matrix<E, dynamic<A>, BO, L>::matrix()
: m_data(0), m_rows(0), m_cols(0)
{
}

template<class E, class A, typename BO, typename L>
matrix<E, dynamic<A>, BO, L>::matrix(int rows, int cols)
: m_data(0), m_rows(0), m_cols(0)
{
  this->resize_fast(rows,cols);
}

template<class E, class A, typename BO, typename L>
matrix<E, dynamic<A>, BO, L>::matrix(const matrix_type& other)
: m_data(0), m_rows(0), m_cols(0)
{
  this->assign(other);
}

template<class E, class A, typename BO, typename L>
matrix<E, dynamic<A>, BO, L>::matrix(matrix_type&& other)
: m_data(0), m_rows(0), m_cols(0)
{
  this->operator=(std::move(other));
}

template<class E, class A, typename BO, typename L> template<class Sub>
matrix<E, dynamic<A>, BO, L>::matrix(const readable_matrix<Sub>& sub)
: m_data(0), m_rows(0), m_cols(0)
{
  this->assign(sub);
}

template<class E, class A, typename BO, typename L>
template<class Array, enable_if_array_t<Array>*>
matrix<E, dynamic<A>, BO, L>::matrix(
  int rows, int cols, const Array& array
  )
: m_data(0), m_rows(0), m_cols(0)
{
  this->resize_fast(rows,cols);
  this->assign(array);
}

template<class E, class A, typename BO, typename L>
template<class Other, int R, int C>
matrix<E, dynamic<A>, BO, L>::matrix(Other const (&array)[R][C])
: m_data(0), m_rows(0), m_cols(0)
{
 this->assign(array);
}

template<class E, class A, typename BO, typename L>
template<class Pointer, enable_if_pointer_t<Pointer>*>
matrix<E, dynamic<A>, BO, L>::matrix(
  int rows, int cols, const Pointer& array
  )
: m_data(0), m_rows(0), m_cols(0)
{
  this->resize_fast(rows,cols);
  this->assign(array);
}

template<class E, class A, typename BO, typename L>
template<class Pointer, enable_if_pointer_t<Pointer>*>
matrix<E, dynamic<A>, BO, L>::matrix(
  const Pointer& array, int rows, int cols
  )
: m_data(0), m_rows(0), m_cols(0)
{
  this->resize_fast(rows,cols);
  this->assign(array);
}

template<class E, class A, typename BO, typename L>
matrix<E, dynamic<A>, BO, L>::~matrix()
{
  typedef typename allocator_traits::size_type size_type;
  int n = this->m_rows*this->m_cols;
  this->destruct(this->m_data, n,
    typename std::is_trivially_destructible<E>::type());

  auto allocator = allocator_type();
  allocator_traits::deallocate(allocator, this->m_data, size_type(n));
}



/* Public methods: */

template<class E, class A, typename BO, typename L> auto
matrix<E, dynamic<A>, BO, L>::data() -> pointer
{
  return this->m_data;
}

template<class E, class A, typename BO, typename L> auto
matrix<E, dynamic<A>, BO, L>::data() const -> const_pointer
{
  return this->m_data;
}

template<class E, class A, typename BO, typename L> auto
matrix<E, dynamic<A>, BO, L>::begin() const -> const_pointer
{
  return this->m_data;
}

template<class E, class A, typename BO, typename L> auto
matrix<E, dynamic<A>, BO, L>::end() const -> const_pointer
{
  return this->m_data + this->m_rows*this->m_cols;
}

template<class E, class A, typename BO, typename L> void
matrix<E, dynamic<A>, BO, L>::resize(int rows, int cols)
{
  cml_require(rows >= 0, std::invalid_argument, "rows < 0");
  cml_require(cols >= 0, std::invalid_argument, "cols < 0");

  int n_old = this->m_rows*this->m_cols;
  int n_new = rows*cols;

  /* Short-circuit same size: */
  if(n_new == n_old) {
    this->m_rows = rows;
    this->m_cols = cols;
    return;
  }

  /* Allocator to use: */
  auto allocator = allocator_type();

  /* Allocate the new array: */
  pointer data = this->m_data;
  pointer copy = allocator_traits::allocate(allocator, n_new);
  try {

    /* Destruct elements if necessary: */
    this->destruct(data, n_old,
      typename std::is_trivially_destructible<E>::type());

    /* Copy elements to the new array if necessary: */
    if(data) {
      int to = std::min(n_old, n_new);
      for(pointer src = data, dst = copy; src < data + to; ++ src, ++ dst) {
	allocator_traits::construct(allocator, dst, *src);
      }

      /* Deallocate the old array: */
      allocator_traits::deallocate(allocator, data, n_old);
    }
  } catch(...) {
    allocator_traits::deallocate(allocator, copy, n_new);
    throw;
  }

  /* Save the new array: */
  this->m_data = copy;
  this->m_rows = rows;
  this->m_cols = cols;
}

template<class E, class A, typename BO, typename L> void
matrix<E, dynamic<A>, BO, L>::resize_fast(int rows, int cols)
{
  cml_require(rows >= 0, std::invalid_argument, "rows < 0");
  cml_require(cols >= 0, std::invalid_argument, "cols < 0");

  int n_old = this->m_rows*this->m_cols;
  int n_new = rows*cols;

  /* Short-circuit same size: */
  if(n_new == n_old) {
    this->m_rows = rows;
    this->m_cols = cols;
    return;
  }

  /* Allocator to use: */
  auto allocator = allocator_type();

  /* Allocate the new array: */
  pointer data = this->m_data;
  pointer copy = allocator_traits::allocate(allocator, n_new);
  try {

    /* Destruct elements if necessary: */
    this->destruct(data, n_old,
      typename std::is_trivially_destructible<E>::type());

    /* Deallocate the old array: */
    allocator_traits::deallocate(allocator,data,n_old);
  } catch(...) {
    allocator_traits::deallocate(allocator,copy,n_new);
    throw;
  }

  /* Save the new array: */
  this->m_data = copy;
  this->m_rows = rows;
  this->m_cols = cols;
}


template<class E, class A, typename BO, typename L> auto
matrix<E, dynamic<A>, BO, L>::operator=(const matrix_type& other)
-> matrix_type&
{
  return this->assign(other);
}

template<class E, class A, typename BO, typename L> auto
matrix<E, dynamic<A>, BO, L>::operator=(matrix_type&& other)
-> matrix_type&
{
  /* Ensure deletion of the current array, if any: */
  std::swap(this->m_data, other.m_data);
  std::swap(this->m_rows, other.m_rows);
  std::swap(this->m_cols, other.m_cols);
  /* Note: swap() can't throw here, so this is exception-safe. */

  return *this;
}



/* Internal methods: */

template<class E, class A, typename BO, typename L> void
matrix<E, dynamic<A>, BO, L>::destruct(pointer, int, std::true_type)
{
  /* Nothing to do. */
}

template<class E, class A, typename BO, typename L> void
matrix<E, dynamic<A>, BO, L>::destruct(pointer data, int n, std::false_type)
{
  /* Short-circuit null: */
  if(data == nullptr) return;

  /* Destruct each element: */
  else {
    auto allocator = allocator_type();
    for(pointer e = data; e < data + n; ++ e)
    allocator_traits::destroy(allocator, e);
  }
}


/* readable_matrix interface: */

template<class E, class A, typename BO, typename L> int
matrix<E, dynamic<A>, BO, L>::i_rows() const
{
  return this->m_rows;
}

template<class E, class A, typename BO, typename L> int
matrix<E, dynamic<A>, BO, L>::i_cols() const
{
  return this->m_cols;
}

template<class E, class A, typename BO, typename L> auto
matrix<E, dynamic<A>, BO, L>::i_get(int i, int j) const -> immutable_value
{
  return s_access(*this, i, j, layout_tag());
}


/* writable_matrix interface: */

template<class E, class A, typename BO, typename L> auto
matrix<E, dynamic<A>, BO, L>::i_get(int i, int j) -> mutable_value
{
  return s_access(*this, i, j, layout_tag());
}

template<class E, class A, typename BO, typename L>
template<class Other> auto matrix<E, dynamic<A>, BO, L>::i_put(
  int i, int j, const Other& v
  ) __CML_REF -> matrix_type&
{
  s_access(*this, i, j, layout_tag()) = value_type(v);
  return *this;
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class E, class A, typename BO, typename L>
template<class Other> auto matrix<E, dynamic<A>, BO, L>::i_put(
  int i, int j, const Other& v
  ) && -> matrix_type&&
{
  s_access(*this, i, j, layout_tag()) = value_type(v);
  return (matrix_type&&) *this;
}
#endif

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
