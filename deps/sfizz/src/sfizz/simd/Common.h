// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include <stdint.h>

constexpr uintptr_t ByteAlignmentMask(unsigned N) { return N - 1; }

template<unsigned N, class T>
T* nextAligned(const T* ptr)
{
    return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) + ByteAlignmentMask(N) & (~ByteAlignmentMask(N)));
}

template<unsigned N, class T>
T* prevAligned(const T* ptr)
{
    return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) & (~ByteAlignmentMask(N)));
}

template<unsigned N, class T>
bool unaligned(const T* ptr)
{
    return (reinterpret_cast<uintptr_t>(ptr) & ByteAlignmentMask(N) ) != 0;
}

template<unsigned N, class T, class... Args>
bool unaligned(const T* ptr1, Args... rest)
{
    return unaligned<N>(ptr1) || unaligned<N>(rest...);
}

template<unsigned N, class T, class... Args>
bool willAlign(const T* ptr1, const T* ptr2)
{
    const auto p1 = reinterpret_cast<uintptr_t>(ptr1);
    const auto p2 = reinterpret_cast<uintptr_t>(ptr2);
    return (
        (p1 & ByteAlignmentMask(N)) == (p2 & ByteAlignmentMask(N))
        && ((p1 & ByteAlignmentMask(sizeof(T))) == 0)
    );
}

template<unsigned N, class T, class... Args>
bool willAlign(const T* ptr1, const T* ptr2, Args... rest)
{
    return willAlign<N>(ptr1, ptr2) && willAlign<N>(ptr2, rest...);
}
