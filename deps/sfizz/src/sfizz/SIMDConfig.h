// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once

/**
   Define the following macros, 1 if available, otherwise 0.
   These are allowed to be defined externally, in case automatic detection would
   fail based on the compiler predefinitions.

   - SFIZZ_HAVE_SSE
   - SFIZZ_HAVE_SSE2
   - SFIZZ_HAVE_AVX
   - SFIZZ_HAVE_NEON
 */

#if defined(__GNUC__)
#   if defined(__AVX__)
#       define SFIZZ_DETECT_SSE 1
#       define SFIZZ_DETECT_SSE2 1
#       define SFIZZ_DETECT_AVX 1
#   elif defined(__SSE2__)
#       define SFIZZ_DETECT_SSE 1
#       define SFIZZ_DETECT_SSE2 1
#       define SFIZZ_DETECT_AVX 0
#   elif defined(__SSE__)
#       define SFIZZ_DETECT_SSE 1
#       define SFIZZ_DETECT_SSE2 0
#       define SFIZZ_DETECT_AVX 0
#   else
#       define SFIZZ_DETECT_SSE 0
#       define SFIZZ_DETECT_SSE2 0
#       define SFIZZ_DETECT_AVX 0
#   endif
#   if defined(__ARM_NEON__) || defined(__ARM_NEON)
#       define SFIZZ_DETECT_NEON 1
#   else
#       define SFIZZ_DETECT_NEON 0
#   endif
#elif defined(_MSC_VER)
#   if defined(__AVX__)
#       define SFIZZ_DETECT_SSE 1
#       define SFIZZ_DETECT_SSE2 1
#       define SFIZZ_DETECT_AVX 1
#   elif defined(_M_AMD64) || defined(_M_X64)
#       define SFIZZ_DETECT_SSE 1
#       define SFIZZ_DETECT_SSE2 1
#       define SFIZZ_DETECT_AVX 0
#   elif _M_IX86_FP == 2
#       define SFIZZ_DETECT_SSE 1
#       define SFIZZ_DETECT_SSE2 1
#       define SFIZZ_DETECT_AVX 0
#   elif _M_IX86_FP == 1
#       define SFIZZ_DETECT_SSE 1
#       define SFIZZ_DETECT_SSE2 0
#       define SFIZZ_DETECT_AVX 0
#   endif
// TODO: how to check for NEON on MSVC ARM?
#endif

#ifndef SFIZZ_HAVE_SSE
#   ifdef SFIZZ_DETECT_SSE
#       define SFIZZ_HAVE_SSE SFIZZ_DETECT_SSE
#   else
#       define SFIZZ_HAVE_SSE 0
#   endif
#endif
#ifndef SFIZZ_HAVE_SSE2
#   ifdef SFIZZ_DETECT_SSE2
#       define SFIZZ_HAVE_SSE2 SFIZZ_DETECT_SSE2
#   else
#       define SFIZZ_HAVE_SSE2 0
#   endif
#endif
#ifndef SFIZZ_HAVE_AVX
#   ifdef SFIZZ_DETECT_AVX
#       define SFIZZ_HAVE_AVX SFIZZ_DETECT_AVX
#   else
#       define SFIZZ_HAVE_AVX 0
#   endif
#endif
#ifndef SFIZZ_HAVE_NEON
#   ifdef SFIZZ_DETECT_NEON
#       define SFIZZ_HAVE_NEON SFIZZ_DETECT_NEON
#   else
#       define SFIZZ_HAVE_NEON 0
#   endif
#endif

/**
   Detect one of the following the processor families.

   - SFIZZ_CPU_FAMILY_X86_64
   - SFIZZ_CPU_FAMILY_I386
   - SFIZZ_CPU_FAMILY_AARCH64
   - SFIZZ_CPU_FAMILY_ARM
 */

#if defined(_MSC_VER) && defined(_M_AMD64)
#   define SFIZZ_CPU_FAMILY_X86_64 1
#elif defined(_MSC_VER) && defined(_M_IX86)
#   define SFIZZ_CPU_FAMILY_I386 1
#elif defined(_MSC_VER) && defined(_M_ARM64)
#   define SFIZZ_CPU_FAMILY_AARCH64 1
#elif defined(_MSC_VER) && defined(_M_ARM)
#   define SFIZZ_CPU_FAMILY_ARM 1
#elif defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64)
#   define SFIZZ_CPU_FAMILY_X86_64 1
#elif defined(__i386__) || defined(__i386)
#   define SFIZZ_CPU_FAMILY_I386 1
#elif defined(__aarch64__)
#   define SFIZZ_CPU_FAMILY_AARCH64 1
#elif defined(__arm__) || defined(__arm)
#   define SFIZZ_CPU_FAMILY_ARM 1
#endif
