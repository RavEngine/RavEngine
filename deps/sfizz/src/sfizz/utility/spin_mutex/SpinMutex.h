// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include <atomic>

class SpinMutex {
public:
    void lock() noexcept;
    bool try_lock() noexcept;
    void unlock() noexcept;

private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
};

inline bool SpinMutex::try_lock() noexcept
{
    return !flag_.test_and_set(std::memory_order_acquire);
}

inline void SpinMutex::unlock() noexcept
{
    flag_.clear(std::memory_order_release);
}
