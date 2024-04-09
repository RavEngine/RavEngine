
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <algorithm>
#include <random>
#include <cstdint>
#include <complex>

#include "papi_perf_counter.h"

//#if defined(HAVE_MIPP) && !defined(NO_MIPP)
#if defined(HAVE_MIPP)
#include <mipp.h>

#define MIPP_VECTOR  mipp::vector
#else
#define MIPP_VECTOR  std::vector
#endif

#include "pf_conv_dispatcher.h"
#include "pf_conv.h"


#define TEST_WITH_MIN_LEN     0


MIPP_VECTOR<float> generate_rng_vec(int M, int N = -1, int seed_value = 1)
{
    MIPP_VECTOR<float> v(N < 0 ? M : N);
    std::mt19937 g;
    g.seed(seed_value);
    constexpr float scale = 1.0F / (1.0F + float(INT_FAST32_MAX));
    for (int k = 0; k < M; ++k)
        v[k] = float(int_fast32_t(g())) * scale;
    for (int k = M; k < N; ++k)
        v[k] = 0.0F;
    return v;
}


int bench_oop_core(
        const conv_f_ptrs & conv_arch,
        const float * signal, const int sz_signal,
        const float * filter, const int sz_filter,
        const int blockLen,
        float * y
        )
{
    conv_buffer_state state;
    const auto conv_oop = conv_arch.fp_conv_float_oop;
    int n_out_sum = 0;
    state.offset = 0;
    state.size = 0;
    papi_perf_counter perf_counter(1);
    for (int off = 0; off + blockLen <= sz_signal; off += blockLen)
    {
        state.size += blockLen;
        int n_out = conv_oop(signal, &state, filter, sz_filter, y);
        n_out_sum += n_out;
    }
    return n_out_sum;
}

int bench_inplace_core(
        const conv_f_ptrs & conv_arch,
        float * signal, const int sz_signal,
        const float * filter, const int sz_filter,
        const int blockLen
        )
{
    conv_buffer_state state;
    const auto conv_inplace = conv_arch.fp_conv_float_inplace;
    int n_out_sum = 0;
    state.offset = 0;
    state.size = 0;
    papi_perf_counter perf_counter(1);
    for (int off = 0; off + blockLen <= sz_signal; off += blockLen)
    {
        state.size += blockLen;
        int n_out = conv_inplace(signal, &state, filter, sz_filter);
        n_out_sum += n_out;
    }
    return n_out_sum;
}


int bench_oop(
        const conv_f_ptrs & conv_arch,
        float * buffer,
        const float * signal, const int sz_signal,
        const float * filter, const int sz_filter,
        const int blockLen,
        float * y
        )
{
    conv_buffer_state state;
    const auto conv_oop = conv_arch.fp_conv_float_oop;
    const auto move_rest = conv_arch.fp_conv_float_move_rest;
    int n_out_sum = 0;
    state.offset = 0;
    state.size = 0;
    papi_perf_counter perf_counter(1);
    for (int off = 0; off + blockLen <= sz_signal; off += blockLen)
    {
        move_rest(buffer, &state);
        //memcpy(buffer+state.size, &s[off], B * sizeof(s[0]));
        std::copy(&signal[off], &signal[off+blockLen], buffer+state.size);
        state.size += blockLen;
        int n_out = conv_oop(buffer, &state, filter, sz_filter, &y[n_out_sum]);
        n_out_sum += n_out;
    }
    return n_out_sum;
}

int bench_cx_real_oop(
        const conv_f_ptrs & conv_arch,
        complexf * buffer,
        const float * signal_re, const int sz_signal_re,
        const float * filter, const int sz_filter,
        const int blockLen,
        float * y_re
        )
{
    conv_buffer_state state;
    const auto conv_oop = conv_arch.fp_conv_cplx_float_oop;
    const auto move_rest = conv_arch.fp_conv_cplx_move_rest;
    // interpret buffer, signal and output vector y  as complex data
    complexf * y = reinterpret_cast<complexf *>(y_re);
    const complexf * signal = reinterpret_cast<const complexf *>(signal_re);
    const int sz_signal = sz_signal_re / 2;
    int n_out_sum = 0;
    state.offset = 0;
    state.size = 0;
    papi_perf_counter perf_counter(1);
    for (int off = 0; off + blockLen <= sz_signal; off += blockLen)
    {
        move_rest(buffer, &state);
        //memcpy(buffer+state.size, &s[off], B * sizeof(s[0]));
        std::copy(&signal[off], &signal[off+blockLen], &buffer[state.size]);
        state.size += blockLen;
        int n_out = conv_oop(buffer, &state, filter, sz_filter, &y[n_out_sum]);
        n_out_sum += n_out;
    }
    return n_out_sum;
}


int main(int argc, char *argv[])
{
    // cli defaults:
    // process up to 64 MSample (512 MByte) in blocks of 1 kSamples (=64 kByte) with filterLen 128
    int arch = 0, N = 64 * 1024 * 1024;
    int filterLen = 128, blockLen = 1024;
    int seed_sig = 1, seed_filter = 2;
    bool verbose = false, exitFromUsage = false, showUsage = (argc <= 1);

    for (int i = 1; i < argc; ++i)
    {
        if (i+1 < argc && !strcmp(argv[i], "-a"))
            arch = atoi(argv[++i]);
        else if (i+1 < argc && !strcmp(argv[i], "-n"))
            N = atoi(argv[++i]) * 1024 * 1024;
        else if (i+1 < argc && !strcmp(argv[i], "-f"))
            filterLen = atoi(argv[++i]);
        else if (i+1 < argc && !strcmp(argv[i], "-b"))
            blockLen = atoi(argv[++i]);
        else if (i+1 < argc && !strcmp(argv[i], "-ss"))
            seed_sig = atoi(argv[++i]);
        else if (i+1 < argc && !strcmp(argv[i], "-sf"))
            seed_filter = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-v"))
            verbose = true;
        else if (!strcmp(argv[i], "-h"))
            showUsage = exitFromUsage = true;
        else
            fprintf(stderr, "warning: ignoring/skipping unknown option '%s'\n", argv[i]);
    }

    int num_arch = 0;
    const ptr_to_conv_f_ptrs * conv_arch_ptrs = get_all_conv_arch_ptrs(&num_arch);

    if (verbose)
    {
        fprintf(stderr, "num_arch is %d\n", num_arch);
        for (int a = 0; a < num_arch; ++a)
            if (conv_arch_ptrs[a])
                fprintf(stderr, " arch %d is '%s'\n", a, conv_arch_ptrs[a]->id );
            else
                fprintf(stderr, " arch %d is nullptr !!!\n", a );
        fprintf(stderr, "\n");
    }

    if ( arch < 0 || arch >= num_arch || !blockLen || !N || !filterLen || showUsage )
    {
        fprintf(stderr, "%s [-v] [-a <arch>] [-n <total # of MSamples> [-f <filter length>] [-b <blockLength in samples>]\n", argv[0]);
        fprintf(stderr, "    [-ss <random seed for signal>] [-sf <random seed for filter coeffs>]\n");
        fprintf(stderr, "arch is one of:");
        for (int a = 0; a < num_arch; ++a)
            if (conv_arch_ptrs[a])
                fprintf(stderr, " %d for '%s'%s", a, conv_arch_ptrs[a]->id, (a < num_arch-1 ? ",":"") );
        fprintf(stderr, "\n");
        if ( exitFromUsage || !blockLen || !N || !filterLen || arch < 0 || arch >= num_arch )
            return 0;
    }

    if (verbose)
    {
        #ifdef HAVE_PAPI
        fprintf(stderr, "PAPI is available\n");
        #else
        fprintf(stderr, "PAPI is NOT available!\n");
        #endif
    }
    #if !defined(HAVE_MIPP)
    fprintf(stderr, "MIPP is NOT available!\n");
    #endif

    //int float_simd_size[num_arch];
    int max_simd_size = -1;
    for (int a = 0; a < num_arch; ++a)
    {
        if (conv_arch_ptrs[a])
        {
            const int sz = conv_arch_ptrs[a]->fp_conv_float_simd_size();
            //float_simd_size[a] = sz;
            if (max_simd_size < sz)
                max_simd_size = sz;
            if (verbose)
                fprintf(stderr, "float simd size for '%s': %d\n", conv_arch_ptrs[a]->id, sz);
        }
        //else
        //    float_simd_size[a] = 0;
    }
    //const int max_simd_size = *std::max_element( &float_simd_size[0], &float_simd_size[num_arch] );
    if (verbose)
        fprintf(stderr, "max float simd size: %d\n", max_simd_size);

#if TEST_WITH_MIN_LEN
    filterLen = 2;
#endif

    // round up filter length
    filterLen = max_simd_size * ( ( filterLen + max_simd_size -1 ) / max_simd_size );

#if TEST_WITH_MIN_LEN
    blockLen = 1;
    N = 2 * (3 + filterLen);    // produce 3+1 samples
#endif

    if (!conv_arch_ptrs[arch])
    {
        fprintf(stderr, "Error: architecture %d is NOT available!\n", arch);
        return 1;
    }
    const conv_f_ptrs & conv_arch =  *conv_arch_ptrs[arch];
    if (verbose)
        fprintf(stderr, "arch is using mipp: %d\n", conv_arch.using_mipp);

    fprintf(stderr, "processing N = %d MSamples with block length of %d samples with filter length %d taps on '%s'\n",
        N / (1024 * 1024), blockLen, filterLen, conv_arch.id );

    MIPP_VECTOR<float> s = generate_rng_vec(N + 1, N + 1, seed_sig);
    MIPP_VECTOR<float> y(N + 1, 0.0F);
    MIPP_VECTOR<float> filter = generate_rng_vec(filterLen, filterLen, seed_filter);
    MIPP_VECTOR<float> buffer(blockLen + filterLen + 1, 0.0F);
    MIPP_VECTOR<complexf> buffer_cx(blockLen + filterLen + 1);

#if 1 && TEST_WITH_MIN_LEN
    for (int k = 0; k < N; ++k)
        s[k] = (k+1);
    for (int k = 0; k < filterLen; ++k)
        filter[k] = (k+1);
#endif

    s[N] = 123.0F;
    y[N] = 321.0F;
    buffer[blockLen + filterLen] = 789.0F;
    buffer_cx[blockLen + filterLen].i = 987.0F;

    fprintf(stderr, "\nrunning out-of-place convolution core for '%s':\n", conv_arch.id);
    int n_oop_out = bench_oop_core(conv_arch, s.data(), N, filter.data(), filterLen, blockLen, y.data());
    fprintf(stderr, "oop produced %d output samples\n", n_oop_out);
#if TEST_WITH_MIN_LEN
    for (int k = 0; k < n_oop_out; ++k )
        fprintf(stderr, "y[%2d] = %g\n", k, y[k]);
    fprintf(stderr, "\n");
#endif

    fprintf(stderr, "\nrunning out-of-place convolution for '%s':\n", conv_arch.id);
    n_oop_out = bench_oop(conv_arch, buffer.data(), s.data(), N, filter.data(), filterLen, blockLen, y.data());
    fprintf(stderr, "oop produced %d output samples\n", n_oop_out);
    assert(s[N] == 123.0F);
    assert(y[N] == 321.0F);
    assert(buffer[blockLen + filterLen] == 789.0F);
    assert(buffer_cx[blockLen + filterLen].i == 987.0F);
#if TEST_WITH_MIN_LEN
    for (int k = 0; k < n_oop_out; ++k )
        fprintf(stderr, "y[%2d] = %g\n", k, y[k]);
    fprintf(stderr, "\n");
#endif

    fprintf(stderr, "\nrunning out-of-place complex/real convolution for '%s':\n", conv_arch.id);
    n_oop_out = bench_cx_real_oop(conv_arch, buffer_cx.data(), s.data(), N, filter.data(), filterLen, blockLen, y.data());
    fprintf(stderr, "oop produced %d output samples\n", n_oop_out);
    assert(s[N] == 123.0F);
    assert(y[N] == 321.0F);
    assert(buffer[blockLen + filterLen] == 789.0F);
    assert(buffer_cx[blockLen + filterLen].i == 987.0F);
#if TEST_WITH_MIN_LEN
    fprintf(stderr, "complex output (%d complex samples):\n", n_oop_out);
    for (int k = 0; k < n_oop_out; ++k )
        fprintf(stderr, "y[%2d] = %g  %+g * i\n", k, y[2*k], y[2*k+1]);
    fprintf(stderr, "\n");

    const std::complex<float> * sc = reinterpret_cast< std::complex<float>* >( s.data() );
    const int Nc = N /2;
    fprintf(stderr, "reference with std::complex<float>:\n");
    for (int off = 0; off +filterLen <= Nc; ++off )
    {
        std::complex<float> sum(0.0F, 0.0F);
        for (int k=0; k < filterLen; ++k)
            sum += sc[off+k] * filter[k];
        fprintf(stderr, "yv[%2d] = %g  %+g * i\n", off, sum.real(), sum.imag() );
    }
#endif

    fprintf(stderr, "\nrunning inplace convolution core for '%s':\n", conv_arch.id);
    int n_inp_out = bench_inplace_core(conv_arch, s.data(), N, filter.data(), filterLen, blockLen);
    fprintf(stderr, "inp produced %d output samples\n", n_inp_out);
    assert(s[N] == 123.0F);
    assert(y[N] == 321.0F);
    assert(buffer[blockLen + filterLen] == 789.0F);
    assert(buffer_cx[blockLen + filterLen].i == 987.0F);
#if TEST_WITH_MIN_LEN
    for (int k = 0; k < n_inp_out; ++k )
        fprintf(stderr, "y[%2d] = %g\n", k, s[k]);
    fprintf(stderr, "\n");
#endif

    fprintf(stderr, "\n");
    return 0;
}
