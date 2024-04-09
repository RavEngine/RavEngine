
#include "pf_conv_dispatcher.h"

#if 0
#include <stdio.h>

#define DPRINT(...) fprintf(stderr, __VA_ARGS__)

#else
#define DPRINT(...) do { } while (0)
#endif


#define N_DEFAULT_ARCHES  2
// 0 is "none"
// 1 "dflt"

ptr_to_conv_f_ptrs * get_all_conv_arch_ptrs(int * p_num_arch)
{
    static ptr_to_conv_f_ptrs * all_arches = nullptr;
    static int n_arch = 0;
    if (!all_arches)
    {
        n_arch = N_DEFAULT_ARCHES;
        // @TODO: runtime check if actual CPU supports specific architecture
#if defined(CONV_ARCH_GCC_AMD64)
        static const conv_f_ptrs *conv_arch_ptrs[N_DEFAULT_ARCHES+4] = {0};
        DPRINT("CONV_ARCH_GCC_AMD64: sse3, sse4, avx, avx2\n");
        conv_arch_ptrs[n_arch++] = CONV_FN_ARCH(conv_ptrs, sse3)();
        conv_arch_ptrs[n_arch++] = CONV_FN_ARCH(conv_ptrs, sse4)();
        conv_arch_ptrs[n_arch++] = CONV_FN_ARCH(conv_ptrs, avx) ();
        conv_arch_ptrs[n_arch++] = CONV_FN_ARCH(conv_ptrs, avx2)();
#elif defined(CONV_ARCH_MSVC_AMD64)
        static const conv_f_ptrs *conv_arch_ptrs[N_DEFAULT_ARCHES+3] = {0};
        DPRINT("CONV_ARCH_MSVC_AMD64: sse2, avx, avx2\n");
        conv_arch_ptrs[n_arch++] = CONV_FN_ARCH(conv_ptrs, sse2)();
        conv_arch_ptrs[n_arch++] = CONV_FN_ARCH(conv_ptrs, avx) ();
        conv_arch_ptrs[n_arch++] = CONV_FN_ARCH(conv_ptrs, avx2)();
#elif defined(CONV_ARCH_GCC_ARM32NEON)
        static const conv_f_ptrs *conv_arch_ptrs[N_DEFAULT_ARCHES+3] = {0};
        DPRINT("CONV_ARCH_GCC_ARM32NEON: neon_vfpv4, neon_rpi3_a53\n");
        conv_arch_ptrs[n_arch++] = CONV_FN_ARCH(conv_ptrs, neon_vfpv4)();
        conv_arch_ptrs[n_arch++] = CONV_FN_ARCH(conv_ptrs, neon_rpi3_a53)();
        conv_arch_ptrs[n_arch++] = CONV_FN_ARCH(conv_ptrs, neon_rpi4_a72)();
#elif defined(CONV_ARCH_GCC_AARCH64)
        static const conv_f_ptrs *conv_arch_ptrs[N_DEFAULT_ARCHES+1] = {0};
        DPRINT("CONV_ARCH_GCC_AARCH64: -\n");
        conv_arch_ptrs[n_arch++] = CONV_FN_ARCH(conv_ptrs, armv8a)();
#else
        static const conv_f_ptrs *conv_arch_ptrs[N_DEFAULT_ARCHES] = {0};
        DPRINT("unknown CONV_ARCH: -\n");
#endif
        conv_arch_ptrs[0] = CONV_FN_ARCH(conv_ptrs, none)();
        conv_arch_ptrs[1] = CONV_FN_ARCH(conv_ptrs, dflt)();
        all_arches = conv_arch_ptrs;
    }
    if (p_num_arch)
        *p_num_arch = n_arch;
    return all_arches;
}

