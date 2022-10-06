// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#if defined(SFIZZ_IMPLEMENT_CXX17_ALIGNED_NEW_SUPPORT)
#include <new>
#if defined(_WIN32)
#   include <malloc.h>
#else
#   include <stdlib.h>
#endif

void* operator new(std::size_t count, std::align_val_t al, const std::nothrow_t&) noexcept
{
    void *ptr;
#if defined(_WIN32)
    ptr = ::_aligned_malloc(count, static_cast<std::size_t>(al));
#else
    if (::posix_memalign(&ptr, static_cast<std::size_t>(al), count) != 0)
        ptr = nullptr;
#endif
    return ptr;
}

void operator delete(void* ptr, std::align_val_t, const std::nothrow_t&) noexcept
{
#if defined(_WIN32)
    ::_aligned_free(ptr);
#else
    ::free(ptr);
#endif
}

//------------------------------------------------------------------------------

void* operator new(std::size_t count, std::align_val_t al)
{
    void *ptr = operator new(count, al, std::nothrow);
    if (!ptr)
        throw std::bad_alloc();
    return ptr;
}

void operator delete(void* ptr, std::align_val_t al) noexcept
{
    operator delete(ptr, al, std::nothrow);
}

//------------------------------------------------------------------------------

void* operator new[](std::size_t count, std::align_val_t al)
{
    return operator new(count, al);
}

void* operator new[](std::size_t count, std::align_val_t al, const std::nothrow_t& tag) noexcept
{
    return operator new(count, al, tag);
}

void operator delete[](void* ptr, std::align_val_t al) noexcept
{
    operator delete(ptr, al);
}

void operator delete[](void* ptr, std::align_val_t al, const std::nothrow_t& tag) noexcept
{
    operator delete(ptr, al, tag);
}


#endif // defined(SFIZZ_IMPLEMENT_CXX17_ALIGNED_NEW_SUPPORT)
