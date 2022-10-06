// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "ScopedFTZ.h"
#include "SIMDConfig.h"
#if SFIZZ_HAVE_SSE
#include <pmmintrin.h> // x86 CSR is SSE, but FTZ bits SSE3 and up
#elif SFIZZ_HAVE_NEON
#include "arm_neon.h"
#endif


ScopedFTZ::ScopedFTZ()
{
#if SFIZZ_HAVE_SSE
    unsigned mask = _MM_DENORMALS_ZERO_MASK | _MM_FLUSH_ZERO_MASK;
    registerState = _mm_getcsr();
    _mm_setcsr((static_cast<unsigned>(registerState) & (~mask)) | mask);
#elif SFIZZ_HAVE_NEON && SFIZZ_CPU_FAMILY_ARM
    uintptr_t mask = 1u << 24;
    asm volatile("vmrs %0, fpscr" : "=r"(registerState));
    asm volatile("vmsr fpscr, %0" : : "r"((registerState & (~mask)) | mask));
#elif SFIZZ_HAVE_NEON && SFIZZ_CPU_FAMILY_AARCH64
    uintptr_t mask = 1u << 24;
    asm volatile("mrs %0, fpcr" : "=r"(registerState));
    asm volatile("msr fpcr, %0" : : "r"((registerState & (~mask)) | mask));
#endif
}

ScopedFTZ::~ScopedFTZ()
{
#if SFIZZ_HAVE_SSE
    _mm_setcsr(static_cast<unsigned>(registerState));
#elif SFIZZ_HAVE_NEON && SFIZZ_CPU_FAMILY_ARM
    asm volatile("vmrs %0, fpscr" : : "r"(registerState));
#elif SFIZZ_HAVE_NEON && SFIZZ_CPU_FAMILY_AARCH64
    asm volatile("mrs %0, fpcr" : : "r"(registerState));
#endif
}
