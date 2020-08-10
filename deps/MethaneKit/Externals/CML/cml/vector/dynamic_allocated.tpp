/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_VECTOR_DYNAMIC_ALLOCATED_TPP
#error "vector/dynamic_allocated.tpp not included correctly"
#endif

#include <cml/common/exception.h>

namespace cml {

/* dynamic 'structors: */

template<class E, class A>
vector<E, dynamic<A>>::vector()
: m_data(0), m_size(0)
{
}

template<class E, class A>
template<class Int, enable_if_t<std::is_integral<Int>::value>*>
vector<E, dynamic<A>>::vector(Int size)
: m_data(0), m_size(0)
{
  this->resize_fast(int(size));
}

template<class E, class A>
vector<E, dynamic<A>>::vector(const vector_type& other)
: m_data(0), m_size(0)
{
  this->assign(other);
}

template<class E, class A>
vector<E, dynamic<A>>::vector(vector_type&& other)
: m_data(0), m_size(0)
{
  this->operator=(std::move(other));
}

template<class E, class A> template<class Sub>
vector<E, dynamic<A>>::vector(const readable_vector<Sub>& sub)
: m_data(0), m_size(0)
{
  this->assign(sub);
}

template<class E, class A>
template<class Array, enable_if_array_t<Array>*>
vector<E, dynamic<A>>::vector(const Array& array)
: m_data(0), m_size(0)
{
  this->assign(array);
}

template<class E, class A>
template<class Pointer, enable_if_pointer_t<Pointer>*>
vector<E, dynamic<A>>::vector(const Pointer& array, int size)
: m_data(0), m_size(0)
{
  this->resize_fast(size);
  this->assign(array);
}

template<class E, class A>
template<class Pointer, enable_if_pointer_t<Pointer>*>
vector<E, dynamic<A>>::vector(int size, const Pointer& array)
: m_data(0), m_size(0)
{
  this->resize_fast(size);
  this->assign(array);
}

template<class E, class A> template<class Other>
vector<E, dynamic<A>>::vector(std::initializer_list<Other> l)
: m_data(0), m_size(0)
{
  this->assign(l);
}

template<class E, class A>
vector<E, dynamic<A>>::~vector()
{
  typedef typename allocator_traits::size_type size_type;
  int n = this->m_size;
  this->destruct(this->m_data, n,
    typename std::is_trivially_destructible<E>::type());

  auto allocator = allocator_type();
  allocator_traits::deallocate(allocator, this->m_data, size_type(n));
}



/* Public methods: */

template<class E, class A> auto
vector<E, dynamic<A>>::data() -> pointer
{
  return this->m_data;
}

template<class E, class A> auto
vector<E, dynamic<A>>::data() const -> const_pointer
{
  return this->m_data;
}

template<class E, class A> auto
vector<E, dynamic<A>>::begin() const -> const_pointer
{
  return this->m_data;
}

template<class E, class A> auto
vector<E, dynamic<A>>::end() const -> const_pointer
{
  return this->m_data + this->m_size;
}

template<class E, class A> void
vector<E, dynamic<A>>::resize(int n)
{
  cml_require(n >= 0, std::invalid_argument, "size < 0");

  /* Short-circuit same size: */
  if(n == this->m_size) return;

  /* Allocator to use: */
  auto allocator = allocator_type();

  /* Allocate the new array: */
  pointer data = this->m_data;
  int size = this->m_size;
  pointer copy = allocator_traits::allocate(allocator, n);
  try {

    /* Destruct elements if necessary: */
    this->destruct(data, size,
      typename std::is_trivially_destructible<E>::type());

    /* Copy elements to the new array if necessary: */
    if(data) {
      int to = std::min(size, n);
      for(pointer src = data, dst = copy; src < data + to; ++ src, ++ dst) {
	allocator_traits::construct(allocator, dst, *src);
      }

      /* Deallocate the old array: */
      allocator_traits::deallocate(allocator, data, size);
    }
  } catch(...) {
    allocator_traits::deallocate(allocator,copy,n);
    throw;
  }

  /* Save the new array: */
  this->m_data = copy;
  this->m_size = n;
}

template<class E, class A> void
vector<E, dynamic<A>>::resize_fast(int n)
{
  cml_require(n >= 0, std::invalid_argument, "size < 0");

  /* Short-circuit same size: */
  if(n == this->m_size) return;

  /* Allocator to use: */
  auto allocator = allocator_type();

  /* Allocate the new array: */
  pointer data = this->m_data;
  int size = this->m_size;
  pointer copy = allocator_traits::allocate(allocator, n);
  try {

    /* Destruct elements if necessary: */
    this->destruct(data, size,
      typename std::is_trivially_destructible<E>::type());

    /* Deallocate the old array: */
    allocator_traits::deallocate(allocator,data,size);
  } catch(...) {
    allocator_traits::deallocate(allocator,copy,n);
    throw;
  }

  /* Save the new array: */
  this->m_data = copy;
  this->m_size = n;
}


template<class E, class A> auto
vector<E, dynamic<A>>::operator=(const vector_type& other) -> vector_type&
{
  return this->assign(other);
}

template<class E, class A> auto
vector<E, dynamic<A>>::operator=(vector_type&& other) -> vector_type&
{
  /* Ensure deletion of the current array, if any: */
  std::swap(this->m_data, other.m_data);
  std::swap(this->m_size, other.m_size);
  /* Note: swap() can't throw here, so this is exception-safe. */

  return *this;
}



/* Internal methods: */

template<class E, class A> void
vector<E, dynamic<A>>::destruct(pointer, int, std::true_type)
{
  /* Nothing to do. */
}

template<class E, class A> void
vector<E, dynamic<A>>::destruct(pointer data, int n, std::false_type)
{
  /* Short-circuit null: */
  if(data == nullptr) return;

  /* Destruct each element: */
  else {
    auto allocator = allocator_type();
    for(pointer e = data; e < data + n; ++ e)
      allocator_traits::destroy(allocator,e);
  }
}


/* readable_vector interface: */

template<class E, class A> int
vector<E, dynamic<A>>::i_size() const
{
  return this->m_size;
}

template<class E, class A> auto
vector<E, dynamic<A>>::i_get(int i) const -> immutable_value
{
  return this->m_data[i];
}


/* writable_vector interface: */

template<class E, class A> auto
vector<E, dynamic<A>>::i_get(int i) -> mutable_value
{
  return this->m_data[i];
}

template<class E, class A> template<class Other> auto
vector<E, dynamic<A>>::i_put(int i, const Other& v) __CML_REF -> vector_type&
{
  this->m_data[i] = value_type(v);
  return *this;
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class E, class A> template<class Other> auto
vector<E, dynamic<A>>::i_put(int i, const Other& v) && -> vector_type&&
{
  this->m_data[i] = value_type(v);
  return (vector_type&&) *this;
}
#endif

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
