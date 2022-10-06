// -*- C++ -*-
// SPDX-License-Identifier: BSL-1.0
//
//          Copyright Jean Pierre Cimalando 2018-2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "../../allocator"
#include <stdlib.h>

namespace jsl {

inline void *stdc_allocator_traits::allocate(std::size_t n)
{
    return ::malloc(n);
}

inline void stdc_allocator_traits::deallocate(void *p, std::size_t)
{
    ::free(p);
}

}  // namespace jsl
