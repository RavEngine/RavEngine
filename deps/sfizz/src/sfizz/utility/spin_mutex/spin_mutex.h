// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct spin_mutex_ spin_mutex_t;

spin_mutex_t* spin_mutex_create();
void spin_mutex_destroy(spin_mutex_t* mtx);
void spin_mutex_lock(spin_mutex_t* mtx);
void spin_mutex_unlock(spin_mutex_t* mtx);
bool spin_mutex_trylock(spin_mutex_t* mtx);

#if defined(__cplusplus)
} // extern "C"
#endif
