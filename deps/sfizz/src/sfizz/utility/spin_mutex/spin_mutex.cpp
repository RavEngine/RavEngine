// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "spin_mutex.h"
#include "SpinMutex.h"

struct spin_mutex_ {
    SpinMutex mtx;
};

spin_mutex_t* spin_mutex_create()
{
    return new spin_mutex_t;
}

void spin_mutex_destroy(spin_mutex_t* mtx)
{
    delete mtx;
}

void spin_mutex_lock(spin_mutex_t* mtx)
{
    mtx->mtx.lock();
}

void spin_mutex_unlock(spin_mutex_t* mtx)
{
    mtx->mtx.unlock();
}

bool spin_mutex_trylock(spin_mutex_t* mtx)
{
    return mtx->mtx.try_lock();
}
