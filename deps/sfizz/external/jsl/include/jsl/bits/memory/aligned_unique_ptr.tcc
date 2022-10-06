// -*- C++ -*-
// SPDX-License-Identifier: BSL-1.0
//
//          Copyright Jean Pierre Cimalando 2018-2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once
#include "../../memory"

namespace jsl {

template <class T, std::size_t Al>
void aligned_ptr_delete<T, Al>::operator()(T *p) const noexcept
{
    aligned_allocator<T, Al> allocator;
    allocator.destroy(p);
    allocator.deallocate(p, 0);
}

template <class T, std::size_t Al, class... Args>
aligned_unique_ptr<typename make_aligned_traits<T>::single_element, Al>
make_aligned(Args &&... args)
{
    struct allocation_delete {
        void operator()(T *p) const noexcept
        {
            aligned_allocator<T, Al> allocator;
            allocator.destroy(p);
        }
    };
    aligned_allocator<T, Al> allocator;
    std::unique_ptr<T, allocation_delete> allocation(allocator.allocate(1, nullptr));
    allocator.construct(allocation.get(), std::forward<Args>(args)...);
    return aligned_unique_ptr<T, Al>(allocation.release());
}

} // namespace jsl
