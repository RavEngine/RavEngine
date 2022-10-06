// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once

#if !defined(NDEBUG) || defined(SFIZZ_ENABLE_RELEASE_ASSERT)
#include <iostream>

// Break in source code
#if defined(_WIN32) && defined(_MSC_VER)
#pragma intrinsic(__debugbreak)
#define debugBreak() __debugbreak()
#elif defined(_WIN32)
#define debugBreak() __debugbreak()
#elif defined(__x86_64__) || defined(__i386__)
#define debugBreak() asm volatile("int3")
#elif defined(__clang__) && ((__clang_major__ > 3) || (__clang_major__ == 3 && __clang_minor__ >= 2))
#define debugBreak() __builtin_debugtrap()
#elif defined(__GNUC__)
#define debugBreak() __builtin_trap()
#else
#include <csignal>
#define debugBreak() ::raise(SIGTRAP)
#endif

// Assert stuff
#define ASSERTFALSE                                                              \
    do {                                                                         \
        std::cerr << "Assert failed at " << __FILE__ << ":" << __LINE__ << '\n'; \
        debugBreak();                                                            \
    } while (0)

#define ASSERT(expression)                                          \
    do {                                                            \
        if (!(expression)) {                                        \
            std::cerr << "Assert failed: " << #expression << '\n';  \
            ASSERTFALSE;                                            \
        }                                                           \
    } while (0)

#define CHECKFALSE                                                              \
    do {                                                                        \
        std::cerr << "Check failed at " << __FILE__ << ":" << __LINE__ << '\n'; \
    } while (0)

#define SFIZZ_CHECK(expression)                                         \
    do {                                                          \
        if (!(expression)) {                                      \
            std::cerr << "Check failed: " << #expression << '\n'; \
            CHECKFALSE;                                           \
        }                                                         \
    } while (0)

#else // NDEBUG

#define ASSERTFALSE do {} while (0)
#define ASSERT(expression) do {} while (0)
#define CHECKFALSE do {} while (0)
#define SFIZZ_CHECK(expression) do {} while (0)

#endif

// Debug message
#include <iostream>
#if !defined(NDEBUG) || defined(SFIZZ_ENABLE_RELEASE_DBG)
#include <iomanip>
#define DBG(ostream) do { std::cerr << std::fixed << std::setprecision(2) << ostream << '\n'; } while (0)
#else
#define DBG(ostream) do { if (0) { std::cerr << ostream; } } while (0)
#endif
