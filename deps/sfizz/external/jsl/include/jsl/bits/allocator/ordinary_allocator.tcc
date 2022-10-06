// -*- C++ -*-
// SPDX-License-Identifier: BSL-1.0
//
//          Copyright Jean Pierre Cimalando 2018-2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "../../allocator"
#include <new>

namespace jsl {

template <class T, class Traits>
inline T *ordinary_allocator<T, Traits>::address(T &x) const noexcept
{
    return &x;
}

template <class T, class Traits>
inline const T *ordinary_allocator<T, Traits>::address(const T &x) const noexcept
{
    return &x;
}

template <class T, class Traits>
T *ordinary_allocator<T, Traits>::allocate(std::size_t n, const void *)
{
    T *ptr = (T *)Traits::allocate(n * sizeof(T));
    if (!ptr)
        throw std::bad_alloc();
    return ptr;
}

template <class T, class Traits>
void ordinary_allocator<T, Traits>::deallocate(T *p, std::size_t n) noexcept
{
    Traits::deallocate(p, n * sizeof(T));
}

template <class T, class Traits>
std::size_t ordinary_allocator<T, Traits>::max_size() const noexcept
{
    return std::numeric_limits<std::size_t>::max() / sizeof(T);
}

template <class T, class Traits>
template <class U, class... Args>
void ordinary_allocator<T, Traits>::construct(U *p, Args &&...args)
{
    ::new((void *)p) U(std::forward<Args>(args)...);
}

template <class T, class Traits>
template <class U>
void ordinary_allocator<T, Traits>::destroy(U *p)
{
    p->~U();
}

}  // namespace jsl
