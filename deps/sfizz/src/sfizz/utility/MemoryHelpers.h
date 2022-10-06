// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include <jsl/allocator>
#include <memory>

namespace sfz {

/**
 * @brief Allocate and initialize an object of type T, using sufficient
 * alignment for the type.
 * This provides compatibility with systems which do not support aligned-new.
 */
template <class T, size_t A = alignof(T), class... Args>
T* alignedNew(Args&& ...args)
{
    using allocator = jsl::aligned_allocator<T, A>;
    struct deleter {
        void operator()(T* x) const noexcept { allocator().deallocate(x, sizeof(T)); }
    };
    std::unique_ptr<T, deleter> ptr { allocator().allocate(1) };
    allocator().construct(ptr.get(), std::forward<Args>(args)...);
    return ptr.release();
}

/**
 * @brief Deinitialize and deallocate the aligned object of type T. It must have
 * been previously instantiated by alignedNew.
 */
template <class T, size_t A = alignof(T)>
void alignedDelete(T* ptr)
{
    using allocator = jsl::aligned_allocator<T, A>;
    if (ptr) {
        allocator().destroy(ptr);
        allocator().deallocate(ptr, 1);
    }
}

/**
 * @brief Deletes an object using alignedDelete.
 */
template <class T, size_t A = alignof(T)>
struct aligned_deleter {
    void operator()(T* x) const noexcept { alignedDelete<T, A>(x); }
};

/**
 * @brief Unique pointer which uses alignedDelete as the cleanup function.
 */
template <class T, size_t A = alignof(T)>
using aligned_unique_ptr = std::unique_ptr<T, aligned_deleter<T, A>>;

} // namespace sfz
