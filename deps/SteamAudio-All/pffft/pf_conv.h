#pragma once

/* pf_conv.h/.cpp implements linear "slow" convolution.
 * this code is primarily for test/demonstration of runtime dispatching.
 * each "kernel" is compiled with different compiler/architecture options,
 * that activates different implementations in the MIPP headers.
 *
 * the dispatcher library 'pf_conv_dispatcher' collects (links agains)
 * all the pf_conv_arch_<opt> libraries ..
 * and provides the  get_all_conv_arch_ptrs() function,
 * which delivers an array of pointers to the struct (conv_f_ptrs)
 * containing the function pointers for the different implementations.
 *
 * requirement(s):
 * - installed MIPP headers
 * - compiler definitions for the different architecture types:
 *   see CMakeLists.txt CONV_ARCH_MSVC_AMD64, CONV_ARCH_GCC_ARM32NEON, ..
 * - one cmake library target pf_conv_arch_<opt> for each architecture option.
 *   each one gets it's specific  architecture/compiler  options
 *    utilizing the target_set_cxx_arch_option() macro in the CMakeLists.txt
 */

#include "pf_cplx.h"

#if defined(_MSC_VER)
#  define RESTRICT __restrict
#elif defined(__GNUC__)
#  define RESTRICT __restrict
#else
#  define RESTRICT
#endif


struct conv_buffer_state
{
    int offset; // sample index where data (to process) starts
    int size;   // actual - or previous - size in amount of samples from buffer start (NOT offset)
};

// declare provided function pointer types

typedef const char * (*f_conv_id)();

typedef int  (*f_conv_float_simd_size)();

typedef void (*f_conv_float_move_rest)(float * RESTRICT s, conv_buffer_state * RESTRICT state);
typedef void (*f_conv_cplx_move_rest)(complexf * RESTRICT s, conv_buffer_state * RESTRICT state);

typedef int  (*f_conv_float_inplace)(
        float * RESTRICT s, conv_buffer_state * RESTRICT state,
        const float * RESTRICT filter, const int sz_filter
        );

typedef int  (*f_conv_float_oop)(
        const float * RESTRICT s, conv_buffer_state * RESTRICT state,
        const float * RESTRICT filter, const int sz_filter,
        float * RESTRICT y
        );

typedef int  (*f_conv_cplx_float_oop)(
        const complexf * RESTRICT s, conv_buffer_state * RESTRICT state,
        const float * RESTRICT filter, const int sz_filter,
        complexf * RESTRICT y
        );


// struct with the provided function pointers
struct conv_f_ptrs
{
    const char * id;
    const int using_mipp;
    f_conv_id               fp_id;
    f_conv_float_simd_size  fp_conv_float_simd_size;

    f_conv_float_move_rest  fp_conv_float_move_rest;
    f_conv_float_inplace    fp_conv_float_inplace;
    f_conv_float_oop        fp_conv_float_oop;

    f_conv_cplx_move_rest   fp_conv_cplx_move_rest;
    f_conv_cplx_float_oop   fp_conv_cplx_float_oop;
};

typedef const conv_f_ptrs * ptr_to_conv_f_ptrs;

// function pointer type, delivering the struct with the function pointers
typedef const conv_f_ptrs* (*f_conv_ptrs)();


// helper for systematic function names
#define CONV_FN_ARCH(FN, ARCH) FN##_##ARCH

// declare all functions - returning the structs with the function pointers
extern const conv_f_ptrs* CONV_FN_ARCH(conv_ptrs, none)();  // = conv_ptrs_none()
extern const conv_f_ptrs* CONV_FN_ARCH(conv_ptrs, dflt)();  // simd / mipp is activated

extern const conv_f_ptrs* CONV_FN_ARCH(conv_ptrs, sse3)();  // = conv_ptrs_sse3()
extern const conv_f_ptrs* CONV_FN_ARCH(conv_ptrs, sse4)();
extern const conv_f_ptrs* CONV_FN_ARCH(conv_ptrs, avx)();
extern const conv_f_ptrs* CONV_FN_ARCH(conv_ptrs, avx2)();

extern const conv_f_ptrs* CONV_FN_ARCH(conv_ptrs, sse2)();
//extern const conv_f_ptrs* CONV_FN_ARCH(conv_ptrs, avx)();  // already declared
//extern const conv_f_ptrs* CONV_FN_ARCH(conv_ptrs, avx2)(); // already declared

extern const conv_f_ptrs* CONV_FN_ARCH(conv_ptrs, neon_vfpv4)();    // for armv7l / 32-bit ARM
extern const conv_f_ptrs* CONV_FN_ARCH(conv_ptrs, neon_rpi3_a53)();
extern const conv_f_ptrs* CONV_FN_ARCH(conv_ptrs, neon_rpi4_a72)();

extern const conv_f_ptrs* CONV_FN_ARCH(conv_ptrs, armv8a)();  // for aarch64
