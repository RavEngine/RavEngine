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
#if defined(_WIN32)
# include <malloc.h>
#else
# include <stdlib.h>
#endif

namespace jsl {

template <std::size_t Al>
void *aligned_allocator_traits<Al>::allocate(std::size_t n)
{
    static_assert(Al % sizeof(void *) == 0,
                  "alignment must be a multiple of the pointer size");
    static_assert((Al & (~Al + 1)) == Al,
                  "alignment must be a power of two");
#if defined(_WIN32)
    void *p = ::_aligned_malloc(n, Al);
    if (!p)
        throw std::bad_alloc();
    return p;
#else
    void *p;
    if (::posix_memalign(&p, Al, n) != 0)
        throw std::bad_alloc();
    return p;
#endif
}

template <std::size_t Al>
void aligned_allocator_traits<Al>::deallocate(void *p, std::size_t)
{
#if defined(_WIN32)
    ::_aligned_free(p);
#else
    ::free(p);
#endif
}

}  // namespace jsl
