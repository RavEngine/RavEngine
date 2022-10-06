// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "SpinMutex.h"
#include <atomic_queue/defs.h>
#include <thread>

// based on Timur Doumler's implementation advice for spinlocks

void SpinMutex::lock() noexcept
{
    for (int i = 0; i < 5; ++i) {
        if (try_lock())
            return;
    }

    for (int i = 0; i < 10; ++i) {
        if (try_lock())
            return;
        atomic_queue::spin_loop_pause();
    }

    for (;;) {
        for (int i = 0; i < 3000; ++i) {
            if (try_lock())
                return;
            atomic_queue::spin_loop_pause();
            atomic_queue::spin_loop_pause();
            atomic_queue::spin_loop_pause();
            atomic_queue::spin_loop_pause();
            atomic_queue::spin_loop_pause();
            atomic_queue::spin_loop_pause();
            atomic_queue::spin_loop_pause();
            atomic_queue::spin_loop_pause();
            atomic_queue::spin_loop_pause();
            atomic_queue::spin_loop_pause();
        }
        std::this_thread::yield();
    }
}
